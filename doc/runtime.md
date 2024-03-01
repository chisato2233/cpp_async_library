# `runtime` 类

## 概述

`runtime` 类是一个协程调度器，用于管理和调度协程任务的执行。

### 成员函数

- `register_task(co_task<T>&& task) -> register_option`: 注册一个协程任务，并返回注册选项。
- `register_task(F f, Args&&... args)`: 注册一个协程任务，使用给定的函数对象和参数。
- `cancel_task(co_task_base* task_ref)`: 取消一个协程任务。
- `start_task(co_task<T>&& task) -> co_task<T>&`: 注册并启动一个协程任务，并返回任务对象。
- `start_task(uint64_t id) -> co_task_base*`: 根据任务 ID 注册并启动一个协程任务。
- `stop_task(co_task_base* task)`: 停止一个协程任务。
- `stop_task(uint64_t id)`: 根据任务 ID 停止一个协程任务。
- `suspend_task(co_task_base* task)`: 暂停一个协程任务。
- `suspend_task(uint64_t id)`: 根据任务 ID 暂停一个协程任务。
- `resume_task(co_task_base* task)`: 恢复一个暂停的协程任务。
- `resume_task(uint64_t id)`: 根据任务 ID 恢复一个暂停的协程任务。
- `update()`: 更新协程任务状态。
- `task_count() const noexcept -> std::size_t`: 返回当前管理的协程任务数量。

## 结构体 `runtime::register_option`

### 概述

`register_option` 结构体是注册选项类型，用于注册协程任务时提供一些附加选项。

### 成员函数

- `add_predicate(const std::function<bool()>& predicate) &&`: 添加一个谓词函数，当谓词返回 true 时启动注册的任务。
- `add_timer(std::chrono::duration<Rep,Period> duration) &&`: 添加一个定时器，指定时间后启动注册的任务。

## 辅助函数

- `_builtin_call_runtime_start_task(runtime* rt, co_task<T>&& task)`: 内置函数，用于调用 `runtime` 类的 `start_task` 函数。
- `_builtin_call_runtime_stop_task(runtime* rt, co_task_base* task)`: 内置函数，用于调用 `runtime` 类的 `cancel_task` 函数。
- `_builtin_call_runtime_suspend_task(runtime* rt, co_task_base* task)`: 内置函数，用于调用 `runtime` 类的 `suspend_task` 函数。
- `_builtin_call_runtime_resume_task(runtime* rt, co_task_base* task)`: 内置函数，用于调用 `runtime` 类的 `resume_task` 函数。

