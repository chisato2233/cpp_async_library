#pragma once
#include"../delegate.h"
#include"co_task.h"
#include"runtime.h"

namespace cst::async{

	template<class Awa> concept _no_suspend_awaitable = requires(Awa a) {
		{a.await_ready()} -> std::convertible_to<bool>;
		{a.await_resume()};
	};

	template<class Awa> concept awaitable = _no_suspend_awaitable<Awa> && requires (Awa a) {
		{ a.await_suspend(std::declval<std::coroutine_handle<>>()) };
	};

	template<class Awa> concept standard_awaitable = _no_suspend_awaitable<Awa> && requires(Awa a) {
		{ a.await_suspend(std::declval<std::coroutine_handle<task_promise_base>>()) };
	};

	

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

	struct _multi_awaiter_tag{};
	template<class T>
	concept multi_awaitable =	standard_awaitable<T> && 
								std::is_base_of_v<_multi_awaiter_tag, T>;

	template<standard_awaitable... A> struct multi_awaiter : _multi_awaiter_tag{

		multi_awaiter(A&&... args) :args_(std::forward_as_tuple(args...)) {
			  
		}

		std::tuple<A...> await_resume() {
			return std::move(args_);
		}

		bool await_ready()const {
			return (std::get<A>(args_).await_ready() && ...);
		}

		template<standard_promise P>
		void await_suspend(std::coroutine_handle<P> caller) {
			auto rt = caller.promise().runtime_ref;
			auto tk = caller.promise().get_task();
			rt->suspend_task(tk.get());

			auto id = rt->register_task([](auto rt, auto tk, auto args) -> co_task<> {
				if (tk) rt->resume_task(tk.get());
				co_return;
			}(rt, tk, args_)).add_awaiter(std::get<A>(args_)...);
		}

		std::tuple<A...> args_;
	};


	
	template<standard_awaitable A, standard_awaitable... B,size_t... Index>
	auto _create_multi_awaiter(A&& awaiter,multi_awaiter<B...>&& other_multi,std::index_sequence<Index...> Ins) {
		return multi_awaiter<A, B...> {
			std::forward<A>(awaiter),
			std::forward<B>(std::get<Index>(other_multi.args_))...
		};
	}

	template<standard_awaitable A, standard_awaitable... B,size_t... Index>
	auto _create_multi_awaiter(multi_awaiter<B...>&& other_multi, A&& awaiter,std::index_sequence<Index...> Ins) {
		return multi_awaiter<B...,A> {
			std::forward<B>(std::get<Index>(other_multi.args_))...,
			std::forward<A>(awaiter)
		};
	}

	template<standard_awaitable A,standard_awaitable... B>
	constexpr auto operator||(A a,multi_awaiter<B...> b) {
		return _create_multi_awaiter(a,b,std::index_sequence_for<B...>{});
	}

	template<standard_awaitable A,standard_awaitable... B>
	constexpr auto operator||(multi_awaiter<B...>&& b,A&& a) {
		return _create_multi_awaiter(
			std::forward<multi_awaiter<B...>>(b),
			std::forward<A>(a),
			std::index_sequence_for<B...>{}
		);
	}. 

	template<standard_awaitable A,standard_awaitable B>
		requires
		!std::is_base_of_v<_multi_awaiter_tag,A>&&
		!std::is_base_of_v<_multi_awaiter_tag,B>
	constexpr auto operator||(A&& a,B&& b) {
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


	template<class Rep ,class Period = std::ratio<1>>
	struct wait_time {
		wait_time(std::chrono::duration<Rep,Period> time):time(time){}
		wait_time(int time):time(std::chrono::seconds(time)){}
		wait_time(double time):time(std::chrono::duration<double>{time}){}


		auto await_resume() { return time; }

		auto await_ready()const->bool {return false;}

		template<class P>
		auto await_suspend(std::coroutine_handle<P> caller) {
			auto rt = caller.promise().runtime_ref;
			auto tk = caller.promise().get_task();

			rt->suspend_task(tk.get());
			auto task = rt->register_task([](auto rt,auto tk)->co_task<> {
				if(tk) rt->resume_task(tk.get());
				co_return;
			}(rt,tk)).add_timer(time);

		}
		std::chrono::duration<Rep,Period> time;
	};

	wait_time(int) -> wait_time<int>;
	wait_time(double) -> wait_time<double>;
	

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
				rt->start_task([](auto rt, auto tk, auto count) ->co_task<> {
					while(--count) {
						co_await wait_next_frame{};
					}
					rt->resume_task(tk.get());
					co_return;
				}(rt, tk, count_));
				
			} 
		}
	private:
		unsigned count_ = 1;

	};

}

