# Cpp-async-library

这是一个基于C++20协程的纯头文件协程库。
## 快速开始

### 1. 初始化和运行时配置
- 包含头文件`cst_async.h`以访问协程库。
- 创建`cst::async::runtime`实例，该实例将作为协程的运行环境。

### 2. 定义协程函数
在定义协程函数时，您需要返回一个`cst::async::co_task<T>`类型的对象，其中`T`是您希望协程返回的类型。如果您的协程不需要返回任何值，可以使用`void`作为`T`的类型。协程内部可以使用`co_await`来等待其他协程或表达式，`co_return`用于返回结果（对于返回类型`void`的协程，则只需结束函数）。

#### 简单协程定义：

```cpp
auto exampleCoroutine(int value) -> cst::async::co_task<int> {
    // 做一些异步操作，例如等待时间
    co_await cst::async::wait_time{std::chrono::seconds{1}};
    // 返回处理后的值
    co_return value * 2;
}
```
### 3. 启动协程
通过`runtime->start_task(exampleCoroutine())`来启动协程.

### 4. 启动运行环境
在程序应用程序主循环中添加`runtime->update()`,循环调度器

## 示例代码
``` cpp
#include"cst_async.h"
#include "cst_async.h"
#include <iostream>

using namespace cst;


// 定义一个协程函数，每帧加载协程
auto loadFrame() -> async::co_task<> {
    while (true) {
        
        std::cout << '>';
        co_await async::wait_next_frame{10000}; //等待接下来的一万帧
    }
}

// 定义一个协程函数，每秒打印数字 1到10
auto printNumbers() -> async::co_task<> {
    for (int i = 0; i <= 10; ++i) {
        co_await async::wait_time{1}; // 等待下一秒
        std::cout << i << '\n';

    }
    co_return; // 结束协程
}


//主协程
auto mainCorotine(async::runtime* rt) -> async::co_task<> {
    // 启动加载帧协程,然后保存协程的引用
    auto& update = rt->start_task(loadFrame());
    // 等待打印数字协程结束
    co_await printNumbers();
    //关闭加载帧协程
    rt->stop_task(update.get_ref());
    co_return;
}

int main() {
    // 创建运行时环境
    auto runtime = std::make_unique<async::runtime>();
    //启动计时器
	timer::start();

    // 启动协程
    runtime->start_task(mainCorotine(runtime.get()));

    // 运行时更新循环，直到所有协程完成
    while (runtime->task_count() != 0) {
        
    	runtime->update(); // 更新运行时状态，推进协程执行
    }

    return 0;
}


/* 结果:
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>0
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>1
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>2
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>3
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>4
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>5
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>6
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>7
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>8
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>9
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>10
 */


```

