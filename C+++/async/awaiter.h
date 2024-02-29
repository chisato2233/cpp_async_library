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
		static void await_resume() {}
		bool await_ready() const { return delegate_->empty(); }
		template<class P>
		void await_suspend(std::coroutine_handle<P> caller) {
			runtime* rt = caller.promise().runtime_ref;
			rt->suspend_task(caller.promise().id);

			delegate_->on_call().push_back([rt, tk = caller.promise().task_ref]{
				rt->resume_task(tk);
			});
		}

	private:
		_delegate_impl<Re, Args...>* delegate_ = nullptr;
	};


	//wait and call the predicate every frame until it return true
	struct wait_until {
		static void await_resume(){}
		auto await_ready() const-> bool { return predicate(); }

		template<class P>
		void await_suspend(std::coroutine_handle<P> caller) {
			auto rt = caller.promise().runtime_ref;
			auto tk = caller.promise().task_ref;
			rt->suspend_task(tk);

			rt->register_task([](auto rt,auto tk)->co_task<> {
				if(tk) rt->resume_task(tk);
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
			auto tk = caller.promise().id;
			rt->suspend_task(tk);

			rt->register_task([](auto rt, auto tk)->co_task<> {
				if (tk) rt->resume_task(tk);
				co_return;
			}(rt, tk)) .add_predicate([&]() {
				return !predicate();
			});
		}

		std::function<bool()> predicate;
	};


	template<class Rep,class Period = std::ratio<1>>
	struct wait_time {
		auto await_resume() { return time; }

		auto await_ready()const->bool {return false;}

		template<class P>
		auto await_suspend(std::coroutine_handle<P> caller) {
			auto rt = caller.promise().runtime_ref;
			auto tk = caller.promise().task_ref;

			rt->suspend_task(tk);
			rt->register_task([](auto rt,auto tk)->co_task<> {
				if(tk) rt->resume_task(tk);
				co_return;
			}(rt,tk)).add_timer(time);
		}
		std::chrono::duration<Rep,Period> time;
	};

	struct wait_next_frame {
		wait_next_frame(unsigned count = 1):count_(count){}

		unsigned await_resume() const{ return count_; }
		auto await_ready()const->bool { return count_ == 0; }

		template<class P>
		auto await_suspend(std::coroutine_handle<P> caller) {
			auto rt = caller.promise().runtime_ref;
			auto tk = caller.promise().task_ref;
			rt->suspend_task(tk);
			rt->resume_task(tk);
		}
	private:
		unsigned count_ = 1;

	};

}

