#pragma once
#include"../delegate.h"
#include"co_task.h"
#include"runtime.h"
#include<variant>



namespace cst::async{

	inline namespace concepts{
		template<class Awa> concept _no_suspend_awaitable = requires(Awa a) {
			{ a.await_ready() } -> std::convertible_to<bool>;
			{ a.await_resume() };
		};

		template<class Awa> concept awaitable = _no_suspend_awaitable<Awa> && requires (Awa a) {
			{ a.await_suspend(std::declval<std::coroutine_handle<>>()) };
		};

		template<class Awa> concept standard_awaitable = _no_suspend_awaitable<Awa> && requires(Awa a) {
			{ a.await_suspend(std::declval<std::coroutine_handle<task_promise_base>>()) };
		};

		struct _multi_awaiter_tag {};
		template<class T>
		concept multi_awaitable = standard_awaitable<T> &&
			std::is_base_of_v<_multi_awaiter_tag, std::remove_cvref_t<T>>;
	}


	inline namespace detail {
		template<_no_suspend_awaitable Awaitable>
		using await_result_t = decltype(std::declval<Awaitable>().await_resume());

		template<typename T>	struct non_void { using type = T; };
		template<>				struct non_void<void> { using type = std::monostate; };
		template<typename T>    using  non_void_t = typename non_void<T>::type;

	}

	//dynamic suspend
	struct suspend {
		bool await_resume()const noexcept { return is_suspend; }
		bool await_ready()const { return !is_suspend; }

		template<standard_promise P>
		void await_suspend(std::coroutine_handle<P> caller)const  {
			auto rt = caller.promise().runtime_ref;
			auto tk = caller.promise().get_task();
			rt->suspend_task(tk.get());
		}

		bool is_suspend = true;
	};


	template<standard_awaitable... A>
	struct multi_awaiter : _multi_awaiter_tag{
		using await_results_variant = std::variant<non_void_t<await_result_t<A>>...>;
		struct multi_await_resume_result {
			size_t return_index;
			await_results_variant resume_result;
		};

		multi_awaiter(A&&... args) : awaitables{std::forward_as_tuple(args...)} {}

		auto await_resume() ->multi_await_resume_result{
			return  {
				.return_index = return_index,
				.resume_result = resume_results
			};
		}

		bool await_ready() {
			
			return await_ready_impl(std::make_index_sequence<sizeof...(A)>());
		}



		template<standard_promise P>
		void await_suspend(std::coroutine_handle<P> caller) {
			caller_runtime = caller.promise().runtime_ref;
			caller_task_base = caller.promise().get_task();
			await_suspend_impl(std::make_index_sequence<sizeof...(A)>());
		}



	private:
		template<size_t... I>
		bool await_ready_impl(std::index_sequence<I...>) {
			return (custom_ready(std::get<I>(awaitables), I) || ...);
		}

		template<size_t... I>
		void await_suspend_impl(std::index_sequence<I...>) {
			std::cout << "multi awaiter: \n";
			(..., custom_await(std::get<I>(awaitables), I));
		}

		template<typename Awaitable>
		bool custom_ready(Awaitable& awaitable, std::size_t index) {
			if(awaitable.await_ready()) {
				return_index = index;
				if constexpr (!std::is_same_v<decltype(awaitable.await_resume()),void>)
					resume_results = awaitable.await_resume();
				return true;
			}
			return false;
		}



		template<typename Awaitable>
		void custom_await(Awaitable& awaitable, std::size_t index) {
			await_suspend_tasks[index] = 
				caller_task_base ->
			derived_task(start_awaitable_task(awaitable, index)).get();
		}

		
		auto start_awaitable_task(standard_awaitable auto& awatable, size_t index) -> co_task<>{
			using resume_type = decltype(awatable.await_resume());

			if constexpr (!std::is_same_v<resume_type, void>) {
				auto resume = co_await awatable;
				resume_results.template emplace<resume_type>(resume);
				stop_others(index);
				co_return;
			}
			else {
				co_await awatable;
				stop_others(index);
			}
		}

		void stop_others(size_t i) {
			
			for(size_t k = 0; k<sizeof...(A); k++) {
				if (k == i)continue;
				caller_runtime->stop_task(await_suspend_tasks[k]);

			}
			return_index = i;
		}

	public:
		std::tuple<A...> awaitables;
		await_results_variant resume_results;

		runtime* caller_runtime;
		ptr<co_task_base> caller_task_base;

		std::array<co_task_base*, sizeof...(A)> await_suspend_tasks;
		size_t return_index = -1;
		
	};



	template<standard_awaitable A, standard_awaitable... B, size_t... Index>
	auto _create_multi_awaiter(A&& awaiter, multi_awaiter<B...>&& other_multi, std::index_sequence<Index...> Ins) {
		return multi_awaiter<A, B...> {
			std::forward<A>(awaiter),
				std::forward<B>(std::get<Index>(other_multi.awaitables))...
		};
	}

	template<standard_awaitable A, standard_awaitable... B, size_t... Index>
	auto _create_multi_awaiter(multi_awaiter<B...>&& other_multi, A&& awaiter, std::index_sequence<Index...> Ins) {
		return multi_awaiter<B..., A> {
			std::forward<B>(std::get<Index>(other_multi.awaitables))...,
				std::forward<A>(awaiter)
		};
	}

	
	
	template<standard_awaitable A,standard_awaitable... B>
	constexpr auto operator||(A&& a,multi_awaiter<B...>&& b) {
		return _create_multi_awaiter(
			std::forward<A>(a),
			std::forward<multi_awaiter<B...>>(b),
			std::index_sequence_for<B...>{}
		);
	}

	template<standard_awaitable A,standard_awaitable... B>
	constexpr auto operator||(multi_awaiter<B...>&& b,A&& a) {
		return _create_multi_awaiter(
			std::forward<multi_awaiter<B...>>(b),
			std::forward<A>(a),
			std::index_sequence_for<B...>{}
		);
	}

	template<standard_awaitable A, standard_awaitable B>
		requires
		(!std::is_base_of_v<_multi_awaiter_tag, A>) &&
		(!std::is_base_of_v<_multi_awaiter_tag, B>)
		constexpr auto operator||(A&& a, B&& b) {
		return multi_awaiter<A, B>{std::forward<A>(a), std::forward<B>(b)};
	}


	template<class Re, class... Args>
	struct _delegate_awaiter {

		_delegate_awaiter(_delegate_impl<Re,Args...>* delegate_ptr) :delegate_{delegate_ptr}{}

		auto await_resume()noexcept -> std::tuple<Args...> {
			return std::move(args_);
		}

		bool await_ready() const { return delegate_->empty(); }
		template<class P>
		void await_suspend(std::coroutine_handle<P> caller) {
			auto rt = caller.promise().runtime_ref;
			auto tk = caller.promise().get_task();

			rt->suspend_task(tk.get());

			auto id = delegate_->on_call().add( [=,this](auto&&... values){
				rt->resume_task(tk.get());
				args_ = std::make_tuple(std::forward<decltype(values)>(values)...);
			});
			delegate_->on_call().delay_remove(id);
		}

	private:
		_delegate_impl<Re, Args...>* delegate_ = nullptr;
		std::tuple<Args...> args_;
	};

	template<class Re,class... Args,standard_awaitable Awaitable>
	constexpr auto operator||(_delegate_impl<Re,Args...>& delegate,Awaitable awaitable) {
		return _delegate_awaiter<Re, Args...>{&delegate} || awaitable;
	}

	template<class Re,class... Args,standard_awaitable Awaitable>
	constexpr auto operator||(Awaitable awaitable,_delegate_impl<Re,Args...>& delegate) {
		return awaitable ||_delegate_awaiter<Re, Args...>{&delegate};
	}






	template<class T>
	struct _co_task_awaiter {
		co_task<T>* this_task;

		bool await_ready()const noexcept { return !this_task || this_task->done(); }

		template<standard_promise Promise>
		void await_suspend(std::coroutine_handle<Promise> caller) {
			auto rt = caller.promise().runtime_ref;
			auto caller_task = caller.promise().get_task();
			std::cout << std::format("task {} call task {} \n", caller_task->task_id(), this_task->task_id());
			if (rt && !this_task->has_runtime())
				caller_task->derived_task(*this_task);


		}

		T await_resume() {
			if constexpr (std::is_same_v<void, T>) {
					return this_task->get_coroutine_handle().promise().return_void();
			}
			else {
					return this_task->get_coroutine_handle().promise().return_value();
			}
		}
		
	};

	template<class T,standard_awaitable Awaitable>
	constexpr auto operator||(co_task<T>&& task,Awaitable awaitable) {
		return _co_task_awaiter<T>{&task} || awaitable;
	}

	template<class T, standard_awaitable Awaitable>
	constexpr auto operator||(Awaitable awaitable,co_task<T>&& task) {
		return std::forward<Awaitable>(awaitable) || _co_task_awaiter<T>{&task};
	}










	//wait and call the predicate every frame until it return true
	struct wait_until {
		auto await_ready() const-> bool { return predicate(); }

		void await_resume() {

		}

		template<class P>
		void await_suspend(std::coroutine_handle<P> caller) {
			auto rt = caller.promise().runtime_ref;
			auto tk = caller.promise().get_task();
			rt->suspend_task(tk.get());

			rt->register_task([](auto rt,auto tk)->co_task<> {
				if(tk) rt->resume_task(tk.get());
				co_return;
			}(rt,tk)).add_predicate(predicate);
		}
		std::function<bool()> predicate;
	};

	//wait and call the predicate every frame while it return true, otherwise resume the task
	struct wait_while {
		bool await_ready()const { return !predicate(); }
		void await_resume() {
			
		}
		template<class P>
		void await_suspend(std::coroutine_handle<P> caller) {
			auto rt = caller.promise().runtime_ref;
			auto tk = caller.promise().get_task();
			rt->suspend_task(tk.get());

			rt->register_task([](auto rt, auto tk)->co_task<> {
				if (tk) rt->resume_task(tk.get());
				co_return;
			}(rt, tk)).add_predicate([&]() {
				return !predicate();
			});
		}

		std::function<bool()> predicate;
	};


	template<class Rep ,class Period>
	struct wait_time {
		wait_time(std::chrono::duration<Rep,Period> time):time(time){}
		wait_time(int time):time(std::chrono::seconds(time)){}
		wait_time(double time):time(std::chrono::duration<double,std::milli>{time*1000}){}


		auto await_resume() { return time; }

		auto await_ready()const->bool {return false;}

		template<class P>
		auto await_suspend(std::coroutine_handle<P> caller) {
			auto rt = caller.promise().runtime_ref;
			auto tk = caller.promise().get_task();
			std::cout << std::format("task {} add time {}\n", tk->task_id(),time);

			//老式的方法
			rt->suspend_task(tk.get());
			auto task = rt->register_task([](auto rt,auto tk)->co_task<> {
				if(tk) rt->resume_task(tk.get());
				std::cout << std::format("try to resume task {} \n", tk->task_id());
				co_return;
			}(rt,tk)).add_timer(time);
			tk->on_stop() += [task, rt](auto&&...) {rt->stop_task(task.get()); };
			tk->on_done() += [task, rt](auto&&...) {rt->stop_task(task.get()); };
		}
		std::chrono::duration<Rep,Period> time;
	};

	wait_time(int) -> wait_time<int,std::ratio<1>>;
	wait_time(double) -> wait_time<double,std::milli>;
	

	struct wait_next_frame {
		wait_next_frame(unsigned count = 1):count_(count){}

		unsigned await_resume() const{ return count_; }
		auto await_ready()const->bool { return count_ == 0; }

		template<standard_promise P>
		auto await_suspend(std::coroutine_handle<P> caller) {
			auto rt = caller.promise().runtime_ref;
			auto tk = caller.promise().get_task();
			rt->suspend_task(tk.get());
			if (count_ == 1) rt->resume_task(tk.get());
			else {
				tk->derived_task([](auto count) ->co_task<> {
					while(--count) {
						co_await wait_next_frame{};
					}
					co_return;
				}( count_));
				
			} 
		}
	private:
		unsigned count_ = 1;

	};

}

