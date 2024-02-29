# `cst` 命名空间

## 概述

`cst` 命名空间提供了一个名为 `delegate` 的模板类，用于管理函数指针、函数对象等的委托。

## 类

### `delegate`

`delegate` 类是一个模板类，用于管理函数指针、函数对象等的委托。

#### 成员函数

##### 观察器
- `begin() const noexcept`: 返回委托中函数列表的起始迭代器。
- `end() const noexcept`: 返回委托中函数列表的结束迭代器。
- `empty() const noexcept`: 返回委托是否为空。
- `is_frozen() const noexcept`: 返回委托是否被冻结。
- `size() const noexcept`: 返回委托中函数列表的大小。
- `on_call() noexcept -> delegate<void, Args...>&`: 设置在调用委托时调用下一个委托的函数。

##### 修改器
- `push_back(const func_type& f)`: 将函数对象添加到委托的末尾。
- `push_back(func_type&& f)`: 将函数对象添加到委托的末尾。
- `push_front(const func_type& f)`: 将函数对象添加到委托的开头。
- `push_front(func_type&& f)`: 将函数对象添加到委托的开头。
- `insert(const_iterator it, func_type&& f)`: 在指定位置插入函数对象。
- `emplace_back(func_type&& f)`: 构造并插入一个函数对象到委托的末尾。
- `emplace_front(func_type&& f)`: 构造并插入一个函数对象到委托的开头。

- `erase(const_iterator it)`: 移除指定位置的函数对象。
- `erase(const_iterator begin, const_iterator end)`: 移除指定范围内的函数对象。
- `erase_all()`: 移除所有函数对象。
- `delay_erase(const_iterator it, unsigned call_count = 1)`: 延迟移除指定位置的函数对象。

- `freeze()`: 冻结委托。
- `unfreeze()`: 解除冻结委托。

- `call_all(Args&&... args) -> typename delegate_result<Re>::type`: 调用委托中的所有函数对象。
- `operator()(Args&&... args)`: 重载函数调用运算符，调用 `call_all()`。

- `operator co_await()`: 重载 `co_await` 运算符，用于协程调用。
- `operator bool() const`: 将委托转换为布尔值，判断委托是否为空。

#### 成员变量

- `is_banned`: 是否禁用委托。
- `enable_call_next`: 是否在调用时调用下一个委托的函数。

- `func_list_`: 函数列表。
- `erase_queue_`: 延迟移除队列。
- `call_count_`: 调用计数。
- `is_frozen_`: 是否被冻结。
- `next_delegate_`: 下一个委托。

## 命名空间

### `async`

`async` 命名空间包含异步操作相关的类型和函数。

#### 类

- `_delegate_awaiter`: 用于处理异步委托调用的辅助类。

