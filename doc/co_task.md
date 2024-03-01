# `co_task` 类

## 概述

`co_task` 是一个协程对象模板类，用于管理协程任务的执行。

### 成员类型

- `co_handle`: 表示协程句柄的类型，是 `std::coroutine_handle` 的别名。

### 成员函数

- `co_task(co_handle handle)`: 构造函数，使用给定的协程句柄创建一个协程任务对象。
- `~co_task()`: 析构函数，销毁协程任务对象。
- `operator()()`: 调用运算符重载，执行协程任务。
- `done() -> bool`: 返回协程任务是否已完成。
- `destroy()`: 销毁协程任务句柄。
- `get_coroutine_handle() const noexcept -> co_handle`: 返回协程句柄。
- `get_id() const noexcept -> std::uint64_t`: 返回协程任务的唯一标识符。
- `get_runtime() const noexcept -> runtime*`: 返回协程任务所属的运行时环境。
- `state() const noexcept -> task_state&`: 返回协程任务的状态。
- `on_start() const noexcept -> delegate<task_promise_base, void()>`: 返回协程任务开始时的委托。
- `on_resume() const noexcept -> delegate<task_promise_base, void()>`: 返回协程任务恢复执行时的委托。
- `on_done() const noexcept -> delegate<task_promise_base, void()>`: 返回协程任务完成时的委托。

### 成员函数 operator co_await

- `operator co_await()`: 用于协程等待的运算符重载函数。

## 结构体 `co_task::promise_type`

### 概述

`promise_type` 是 `co_task` 的协程 Promise 类型，用于管理协程的状态和执行过程。

### 成员函数

- `get_return_object() noexcept -> co_task`: 获取协程任务对象。
- `initial_suspend() noexcept -> std::suspend_always`: 返回协程的初始暂停点。
- `final_suspend() noexcept -> std::suspend_never`: 返回协程的最终暂停点。
- `unhandled_exception()`: 处理未捕获的异常。
- `promise_transform(std::suspend_always)`: 转换 Promise 对象状态。
