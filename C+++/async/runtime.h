#pragma once

#include <queue>
#include <array>
#include <functional>
#include <map>

#include"../macro.h"
#include "co_task.h"
#include"timer.h"

namespace cst::async {
	struct runtime{
		struct register_option {
			co_task_base* task_ref;
			runtime* runtime_ref;
			bool is_rewrite = false;

			void add_predicate(const std::function<bool()>& predicate )&& {
				is_rewrite = true;
				runtime_ref->predicate_map_.insert({ ++next_predicate_id_, { predicate,task_ref } });
			}

			template<class Rep, class Period>
			void add_timer(std::chrono::duration<Rep,Period> duration)&& {
				is_rewrite = true;
				
				runtime_ref->timer_queue_.push({ 
					timer::now() + std::chrono::duration_cast<game_time_point>(duration),
					task_ref
				});
			}
		};


		// register a task which well not start immediately, call start_task is a better choice
		// this function will return some options, which you can choose when you want to start the task
		template<class T>
		auto register_task(co_task<T>&& task)->register_option {
			task.promise_base->runtime_ref = this;
			const auto id = task.promise_base->id;
			auto res = task_map.insert({ id, std::make_unique<co_task<T>>(std::move(task)) });
			return {
				.task_ref = res.first->second.get(),
				.runtime_ref = this
			};

		}

		template<std::invocable F, class... Args>
			requires std::is_same_v<std::invoke_result<F, Args&&...>,co_task<>>
		auto register_task(F f,Args&&... args) {
			return register_task(f(std::forward<Args>(args)...));
		}

		//cancel a task will destroy the task immediately, which is dangerous;
		//if you want to stop a task, call stop_task
		void cancel_task(co_task_base* task_ref) {
			auto id = task_ref->promise_base->id;
			auto state = task_ref->promise_base->state;
			if(state==task_state::suspend) suspend_list_.remove(task_ref);
			task_map.erase(id);
		}

		//registrite and start a task
		//do not use the capture of lambda to create a task, it will cause a dangling pointer
		template<class T>
		auto start_task(co_task<T>&& task) -> co_task<T>&{
			co_task_base* task_ref;
			if (task.promise_base->runtime_ref != this)
				task_ref = register_task(std::move(task)).task_ref;
			else task_ref = &task;
			
			_start_task_from_point(task_ref);
			return *reinterpret_cast<co_task<T>*>(task_ref);

		}

		//registrite (if it doesn't) and start a task by task id 
		//do not use the capture of lambda to create a task, it will cause a dangling pointer
		auto start_task(uint64_t id) -> co_task_base* {
			auto res = task_map.find(id);
			if (res == task_map.end()) return nullptr;

			_start_task_from_point(res->second.get());
			return res->second.get();
		}

		//stop a task by point;
		//please 
		void stop_task(co_task_base* task) {
			if (!task) return;
			task->promise_base->state = task_state::stop;
			stop_queue_.push(task);
		}

		//stop a task by id
		void stop_task(uint64_t id) {
			auto res = task_map.find(id);
			if (res == task_map.end()) return;
			stop_task(res->second.get());
		}

		//suspend a task by point
		void suspend_task(co_task_base* task) {
			if (!task) return;
			task->promise_base->state = task_state::suspend;
			suspend_list_.push_back(task);
		}

		//suspend a task by id
		void suspend_task(uint64_t id) {
			auto res = task_map.find(id);
			if (res == task_map.end()) return;
			suspend_task(res->second.get());
		}

		//resume a task by point
		//if the task is not suspend, it will do nothing
		void resume_task(co_task_base* task) {
			if (!task) return;
			if(task->promise_base->state == task_state::suspend) {
				suspend_list_.remove(task);
				_start_task_from_point(task);
			}
		}

		//resume a task by id
		//if the task is not suspend, it will do nothing
		void resume_task(uint64_t id) {
			auto res = task_map.find(id);
			if (res == task_map.end()) return;
			resume_task(res->second.get());
		} 

		void update() {

			// update predicate-------------------------------------------------------------
			for(auto [id, second] : predicate_map_){
				auto [predicate, task] = second;
				if(predicate()) {
					_start_task_from_point(task);
					predicate_map_.erase(id);
				}
			}

			//update time queue--------------------------------------------------------------
			while(!timer_queue_.empty() && timer_queue_.top().first <= timer::now()) {
				const auto task = timer_queue_.top().second;
				timer_queue_.pop();
				_start_task_from_point(task);
			}

			//update task queue---------------------------------------------------------------
			auto& this_task_queue_ = _get_task_queue();
			(++task_queue_flag_) %= task_queue_num;

			while (!this_task_queue_.empty()) {
				const auto task = this_task_queue_.front();
				if (task) {
					auto state = task->promise_base->state;
					auto id = task->promise_base->id;
					
					state == task_state::ready ? task->resume(), 1 :
					state == task_state::suspend ? suspend_list_.push_back(task),1 :
					state == task_state::stop ? stop_queue_.push(task),1 :
					0;

					

				}
				this_task_queue_.pop();
			}

			//update stop queue---------------------------------------------------------------
			while (!stop_queue_.empty()) {
				const auto task = stop_queue_.front();
				if (task && task->promise_base->state == task_state::stop)
					task_map.erase(task->promise_base->id);
				stop_queue_.pop();
			}

			//update suspend queue-------------------------------------------------------------

			for (auto it = suspend_list_.begin(); it != suspend_list_.end(); ++it) {
				bool is_clear = false;
				task_state state;
				if (*it) {
					state = (*it)->promise_base->state;
					
					state == task_state::ready ? _start_task_from_point(*it), is_clear = true :
					state == task_state::suspend ? is_clear = false :
					state == task_state::stop ? stop_queue_.push(*it), is_clear = true
					: false;
				}
				else is_clear = true;

				if (is_clear)
					suspend_list_.erase(it);
			}

		}

		auto task_count()const noexcept->std::size_t {
			return task_map.size();
		}

	private:
		auto _get_task_queue() -> std::queue<co_task_base*>& {
			return task_queue_array_[task_queue_flag_];
		}

		void _start_task_from_point(co_task_base* task_ref) {
			task_ref->promise_base->state = task_state::ready;
			
			_get_task_queue().push(task_ref);
		}



	public:
		inline static constexpr int task_queue_num = 2;
		std::map<uint64_t , uptr<co_task_base>> task_map;

	private:
		inline static int task_queue_flag_ = 0;
		inline static uint64_t next_predicate_id_ = 0;

		std::array<std::queue<co_task_base*>,task_queue_num> task_queue_array_;
		std::queue<co_task_base*> stop_queue_;

		std::list<co_task_base*> suspend_list_;

		std::unordered_map <uint64_t, std::pair<std::function<bool()>,co_task_base*>> predicate_map_;
		std::priority_queue<
			std::pair<game_time_point, co_task_base*>,
			std::vector<std::pair<game_time_point,co_task_base*>>,
			std::greater<>
		> timer_queue_;
	};

	template <class T>
	inline void _builtin_call_runtime_start_task(runtime* rt, co_task<T>&& task) {
		rt->start_task(std::move(task));
	}

	inline void _builtin_call_runtime_stop_task(runtime* rt, co_task_base* task) {
		rt->cancel_task(task);
	}
	inline void _builtin_call_runtime_suspend_task(runtime* rt, co_task_base* task) {
		rt->suspend_task(task);
	}
	inline void _builtin_call_runtime_resume_task(runtime* rt, co_task_base* task) {
		rt->resume_task(task);
	}

}

