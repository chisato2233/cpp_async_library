# `suspend` 结构体

## 概述

`suspend` 结构体是用于协程暂停的可等待体，用于实现动态暂停功能。

### 成员函数

- `bool await_resume() const noexcept`: 获取暂停状态。
- `bool await_ready() const`: 判断是否立即就绪。
- `void await_suspend(std::coroutine_handle<P> caller) const`: 挂起当前协程。

## 成员变量

- `bool is_suspend`: 表示是否暂停。

# `_delegate_awaiter` 结构体

## 概述

`_delegate_awaiter` 结构体是用于委托对象的可等待体，用于等待委托对象不为空时继续执行。

### 成员函数

- `static void await_resume()`: 等待结果的静态函数。
- `bool await_ready() const`: 判断委托对象是否为空。
- `void await_suspend(std::coroutine_handle<P> caller)`: 挂起当前协程，并注册委托对象的调用。

## 成员变量

- `_delegate_impl<Re, Args...>* delegate_`: 委托对象的指针。

# `wait_until` 结构体

## 概述

`wait_until` 结构体是用于等待直到满足条件的可等待体，每帧检查一次条件。

### 成员函数

- `static void await_resume()`: 等待结果的静态函数。
- `bool await_ready() const`: 判断条件是否已满足。
- `void await_suspend(std::coroutine_handle<P> caller)`: 挂起当前协程，并注册条件的检查。

## 成员变量

- `std::function<bool()> predicate`: 条件谓词函数。

# `wait_while` 结构体

## 概述

`wait_while` 结构体是用于等待条件持续满足的可等待体，每帧检查一次条件。

### 成员函数

- `bool await_ready() const`: 判断条件是否不再满足。
- `void await_resume()`: 等待结果的静态函数。
- `void await_suspend(std::coroutine_handle<P> caller)`: 挂起当前协程，并注册条件的检查。

## 成员变量

- `std::function<bool()> predicate`: 条件谓词函数。

# `wait_time` 结构体

## 概述

`wait_time` 结构体是用于等待一定时间的可等待体。

### 成员函数

- `auto await_resume() -> std::chrono::duration<Rep, Period>`: 获取等待的时间。
- `bool await_ready() const`: 判断是否立即就绪。
- `void await_suspend(std::coroutine_handle<P> caller)`: 挂起当前协程，并注册定时器。

## 成员变量

- `std::chrono::duration<Rep, Period> time`: 等待的时间。

# `wait_next_frame` 结构体

## 概述

`wait_next_frame` 结构体是用于等待下一帧的可等待体。

### 构造函数

- `wait_next_frame(unsigned count = 1)`: 构造函数，指定等待的帧数，默认为 1。

### 成员函数

- `unsigned await_resume() const`: 获取等待的帧数。
- `bool await_ready() const`: 判断是否立即就绪。
- `void await_suspend(std::coroutine_handle<P> caller)`: 挂起当前协程，等待下一帧。

## 成员变量

- `unsigned count_`: 等待的帧数。

