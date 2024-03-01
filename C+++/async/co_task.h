#pragma once
#include<coroutine>
#include <future>

#include"../delegate.h"
#include"promise_base.h"


namespace cst::async{
	template<class T >struct co_task;

	template<class T>
	inline void _builtin_call_runtime_start_task(runtime*,co_task<T>&& task);
	inline void _builtin_call_runtime_stop_task(runtime*,co_task_base* task);
	inline void _builtin_call_runtime_suspend_task(runtime*,co_task_base* task);
	inline void _builtin_call_runtime_resume_task(runtime*,co_task_base* task);
	inline void _builtin_call_runtime_cancel_task(runtime*,co_task_base* task);

	struct co_task_base {
		co_task_base(task_promise_base* base = nullptr): promise_base_(base) {
			if(promise_base_) {
				promise_base_->task_ref = this;
			}
		}

		virtual ~co_task_base() = default;
		co_task_base(co_task_base&) = delete;

		co_task_base(co_task_base&& other) noexcept:
			co_task_base{ std::exchange(other.promise_base_,nullptr) } {}

		//virtual void start() = 0;
		virtual void resume() = 0;
		virtual void destroy() = 0;
		virtual bool done() = 0;

		virtual auto get_promise()const noexcept->task_promise_base* { return promise_base_; }

		virtual auto task_id()const noexcept->std::uint64_t& { return promise_base_->id; }
		virtual auto task_state()const noexcept->task_state& { return promise_base_->state; }

		auto on_start()const noexcept->delegate<task_promise_base, void()>& { return promise_base_->on_start; }
		auto on_resume()const noexcept->delegate<task_promise_base, void()>& { return promise_base_->on_resume; }
		auto on_stop()const noexcept->delegate<task_promise_base, void()>& { return promise_base_->on_stop; }
		auto on_done()const noexcept->delegate<task_promise_base, void()>& { return promise_base_->on_done; }

		auto get_runtime() const noexcept->runtime& { return *promise_base_->runtime_ref; }
		auto bind_runtime(runtime* new_runtime)noexcept { promise_base_->runtime_ref = new_runtime; }
		auto has_runtime()const noexcept { return promise_base_->runtime_ref != nullptr; }
	private:
		task_promise_base* promise_base_;

	};

	template<class T>
	struct _co_task_promise_res {
		T return_value() {
			return std::move(val_);
		}
	private:
		T val_;
	};
	template<>struct _co_task_promise_res<void>{
		constexpr void return_void() {}
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
				return  co_task{ std::coroutine_handle<promise_type>::from_promise(*this) };
				//res.promise_base = static_cast<task_promise_base*>(this);
				/*task_ref = &res;*/
			}

			constexpr auto initial_suspend() noexcept {return std::suspend_always{};}
			auto final_suspend() noexcept {
				on_done();
				_builtin_call_runtime_stop_task(runtime_ref, task_ref);
				return std::suspend_always{};
			}
			static auto unhandled_exception() { std::terminate(); }

			auto promise_transform(std::suspend_always) {
				if(runtime_ref && task_ref) {
					_builtin_call_runtime_suspend_task(runtime_ref, task_ref);
				}
				return std::suspend_always{};
			}
			~promise_type() = default;
		public:
			


		};
		using co_handle = std::coroutine_handle<promise_type>;

		
		
		//functions-----------------------------------------------------------------------------------
		co_task(co_handle handle) :
			co_task_base{ std::addressof(handle.promise()) },handle_(handle) {}

		~co_task() override {
			if (handle_) 
				handle_.destroy();
		}

		co_task(co_task& other) = delete;
		co_task(co_task&& other)noexcept:
			co_task_base(std::move(other)),
			handle_(std::exchange(other.handle_, {}))
		{}


		void resume()override {
			if (handle_ && !handle_.done()) {
				on_resume()();
				handle_();
			}
		}

		void operator()() { resume(); }
		bool done()override { return handle_.done(); }
		void destroy()override {
			handle_.destroy();
		}

		auto get_coroutine_handle() const noexcept-> co_handle { return handle_; }
		auto get_ref() noexcept->co_task_base* { return reinterpret_cast<co_task_base*>(this); }

		

		//operator co_await-----------------------------------------------------------------------------------
		auto operator co_await() {
			struct _ {
				co_task* this_task;
				
				bool await_ready()const noexcept { return !this_task || this_task->done(); }

				void await_suspend(std::coroutine_handle<promise_type> caller) {
					auto rt = caller.promise().runtime_ref;
					_builtin_call_runtime_suspend_task(rt, caller.promise().task_ref);
					auto caller_task = caller.promise().get_task();
					this_task->on_done() += [caller_task, rt](auto...) {
						_builtin_call_runtime_resume_task(rt, caller_task.get());
					};

					if (rt && !this_task->has_runtime()) 
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