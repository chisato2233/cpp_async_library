#pragma once
#include"../delegate.h"
#include"co_task.h"
#include"runtime.h"

namespace cst::async{

	//dynamic suspend
	struct suspend {
		bool await_resume()const noexcept { return is_suspend; }
		bool await_ready()const { return !is_suspend; }

		template<class P>
		void await_suspend(std::coroutine_handle<P> caller)const  {
			caller.promise().runtime_ref->suspend_task(caller.promise().task_ref);
		}

		bool is_suspend = true;
	};



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

			//auto id = task->task_id();
			//caller.promise().on_stop.push_back([rt,id](auto&&...) {
			//	rt->stop_task(id);
			//});

			//caller.promise().on_done.push_back([rt,id](auto&&...) {
			//	rt->cancel_task(id);
			//});

			// fd
			// caller -> lambda
			// caller -> lambda
			//sdaf
			// fdsf



		}
		std::chrono::duration<Rep,Period> time;
	};

	wait_time(int) -> wait_time<int>;
	wait_time(double) -> wait_time<double>;
	

	struct wait_next_frame {
		wait_next_frame(unsigned count = 1):count_(count){}

		unsigned await_resume() const{ return count_; }
		auto await_ready()const->bool { return count_ == 0; }

		template<class P>
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

