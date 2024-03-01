#pragma once
#include<coroutine>
#include"../delegate.h"

namespace cst::async {
	struct runtime;
	enum class task_state {
		prepared,
		ready,
		suspend,
		stop
	};

	struct task_promise_base {
		task_promise_base() : id(_next_task_id++) {
			
		}

		std::uint64_t id;
		task_state state = task_state::ready;
		runtime* runtime_ref = nullptr;
		struct co_task_base* task_ref = nullptr;

		ptr<co_task_base> get_task() const noexcept;
	public:
		delegate<task_promise_base, void()> on_start{ this };
		delegate<task_promise_base, void()> on_resume{ this };
		delegate<task_promise_base, void()> on_stop{ this };
		delegate<task_promise_base, void()> on_done{ this };
	private:
		inline static std::uint64_t _next_task_id = 0;
	};
};