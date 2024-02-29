#pragma once
#include<coroutine>
#include <future>

#include"../delegate.h"
#include"promise_base.h"


namespace cst::async{
	template<class T >struct co_task;

	template<class T>
	void _builtin_call_runtime_start_task(runtime*,co_task<T>&& task);
	void _builtin_call_runtime_stop_task(runtime*,co_task_base* task);
	void _builtin_call_runtime_suspend_task(runtime*,co_task_base* task);
	void _builtin_call_runtime_resume_task(runtime*,co_task_base* task);

	struct co_task_base {
		co_task_base(task_promise_base* base = nullptr): promise_base(base) {}
		virtual ~co_task_base() = default;
		co_task_base(co_task_base&) = delete;
		co_task_base(co_task_base&& other) noexcept:
			promise_base{ std::exchange(other.promise_base,nullptr) }
		{
			promise_base->task_ref = this;
		}

		virtual void resume() = 0;
		virtual void destroy() = 0;

		virtual bool done() = 0;
		task_promise_base* promise_base;
	};

	template<class T>
	struct _co_task_promise_res {
		T return_value() {
			auto promise = reinterpret_cast<task_promise_base*>(this);
			return std::move(val_);
		}
	private:
		T val_;
	};
	template<>struct _co_task_promise_res<void>{
		void return_void() {
			auto promise = reinterpret_cast<task_promise_base*>(this);
		}
	};

	template<class T>
	struct _select_awaiter_return_type {
		using type = T;
	};

	template<> struct _select_awaiter_return_type<void> { using type = void; };

	template<class T = void>
	struct co_task : co_task_base {

		//promise type-----------------------------------------------------------------------------------
		struct promise_type : task_promise_base , _co_task_promise_res<T>{
			template<typename... Args>  promise_type(Args&&...) {}
			template<typename Obj, typename... Args> promise_type(Obj&& obj, Args&&...) {  }

			auto get_return_object() noexcept -> co_task {
				auto res = co_task{ std::coroutine_handle<promise_type>::from_promise(*this) };
				res.promise_base = static_cast<task_promise_base*>(this);
				task_ref = &res;
				return res;
			}

			constexpr auto initial_suspend() noexcept {return std::suspend_always{};}
			auto final_suspend() noexcept {
				on_done();
				_builtin_call_runtime_stop_task(runtime_ref, task_ref);
				return std::suspend_never{};
			}
			static auto unhandled_exception() { std::terminate(); }

			auto promise_transform(std::suspend_always) {
				if(runtime_ref && task_ref) {
					_builtin_call_runtime_suspend_task(runtime_ref, task_ref);
				}
				return std::suspend_always{};
			}

		public:
			


		};
		using co_handle = std::coroutine_handle<promise_type>;

		
		//functions-----------------------------------------------------------------------------------
		co_task(co_handle handle):handle_(handle){}
		~co_task() { if (handle_) handle_.destroy(); }
		co_task(co_task& other) = delete;
		co_task(co_task&& other)noexcept:
			co_task_base(std::move(other)),
			handle_(std::exchange(other.handle_, {}))
		{}


		void resume()override {
			if (handle_ && !handle_.done()) {
				promise_base->on_resume();
				handle_();
			}
		}

		void operator()() { resume(); }
		bool done()override { return handle_.done(); }
		void destroy()override {
			handle_.destroy();
		}

		auto get_coroutine_handle() const noexcept-> co_handle { return handle_; }
		auto get_id()const noexcept->std::uint64_t { return handle_.promise().id; }
		auto get_runtime() const noexcept->runtime* { return handle_.promise().runtime_ref; }

		auto state()const noexcept->task_state& { return handle_.promise().state; }

		auto on_start()const noexcept->delegate<task_promise_base, void()>& { return handle_.promise().on_start; }
		auto on_resume()const noexcept->delegate<task_promise_base, void()>& { return handle_.promise().on_resume; }
		auto on_done()const noexcept->delegate<task_promise_base, void()>& { return handle_.promise().on_done; }

		//operator co_await-----------------------------------------------------------------------------------
		auto operator co_await() {
			struct _ {
				co_task* this_task;
				
				bool await_ready()const noexcept { return !this_task || this_task->done(); }

				void await_suspend(std::coroutine_handle<promise_type> caller) {
					auto rt = caller.promise().runtime_ref;
					_builtin_call_runtime_suspend_task(rt, caller.promise().task_ref);

					this_task->promise_base->on_done.push_back([caller,rt](auto...) {
						_builtin_call_runtime_resume_task(rt, caller.promise().task_ref);
					});

					if (rt && this_task->get_runtime()==nullptr) 
						_builtin_call_runtime_start_task(rt, std::move(*this_task));
					
				}

				T await_resume() {
					if constexpr(std::is_same_v<void,T>) {
						return this_task->get_coroutine_handle().promise().return_void();
					}
					else {
						return this_task->get_coroutine_handle().promise().return_value();
					}
				}
			};

			return _{.this_task = this};
		}

	private:
		co_handle handle_;
	};


















}