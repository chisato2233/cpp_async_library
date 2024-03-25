#pragma once
#include<coroutine>
#include <format>
#include <future>

#include"../delegate.h"
#include"promise_base.h"
#include "runtime.h"


namespace cst::async{
	template<class T >struct co_task;
	template<class T> struct _co_task_awaiter;


	inline namespace concepts {
		template<class Task> concept standard_task = std::derived_from<std::remove_cvref_t<Task>, struct co_task_base>;
	}


	template<class T>
	void _builtin_call_runtime_start_task(runtime*, co_task<T>&& task);
	void _builtin_call_runtime_stop_task(runtime*, co_task_base* task);
	void _builtin_call_runtime_suspend_task(runtime*, co_task_base* task);
	void _builtin_call_runtime_resume_task(runtime*, co_task_base* task);
	void _builtin_call_runtime_cancel_task(runtime*, co_task_base* task);



	struct co_task_base {
		co_task_base(task_promise_base* base = nullptr): promise_base_(base) {
			if(promise_base_) {
				promise_base_->task_ref = this;
			}

			on_stop() += [](task_promise_base* self) {
				for(auto i:self->derived_task_list) {
					_builtin_call_runtime_stop_task(self->runtime_ref, i.get<0>().get());
				}
				self->derived_task_list.clear();
			};
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

		struct derived_task_option {
			bool is_start_task = true;
			bool is_stop_task = true;
			bool is_suspend_caller = true;
			bool is_resume_caller = true;
		};
		ptr<co_task_base> derived_task(standard_task auto&& task,derived_task_option option = {});

	private:
		task_promise_base* promise_base_;
		
	};

	

	inline namespace details  {
		template<class T>
		struct _co_task_promise_res {
			T return_value() {
				return std::move(val_);
			}
		private:
			T val_;
		};
		template<>struct _co_task_promise_res<void> {
			constexpr void return_void() {}
		};

		template<class T>
		struct _select_awaiter_return_type {
			using type = T;
		};

		template<> struct _select_awaiter_return_type<void> { using type = void; };
	}


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
				std::cout << std::format("task {} done", id);
				on_done();
				_builtin_call_runtime_stop_task(runtime_ref, task_ref);
				return std::suspend_always{};
			}
			auto unhandled_exception() { exception = std::current_exception(); }

			auto promise_transform(std::suspend_always) {
				if(runtime_ref && task_ref) {
					_builtin_call_runtime_suspend_task(runtime_ref, task_ref);
				}
				return std::suspend_always{};
			}
			~promise_type() = default;
		public:
			std::exception_ptr exception;


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
				if(handle_.promise().exception)
					std::rethrow_exception(handle_.promise().exception);
			}
		}

		void operator()() { resume(); }
		bool done()override { return handle_.done(); }
		void destroy()override { handle_.destroy(); }

		auto get_coroutine_handle() const noexcept-> co_handle { return handle_; }
		auto get_ref() noexcept->co_task_base* { return reinterpret_cast<co_task_base*>(this); }
	

		//operator co_await-----------------------------------------------------------------------------------
		auto operator co_await() { return _co_task_awaiter<T>{.this_task = this}; }

	private:
		
		co_handle handle_;
	};



}
