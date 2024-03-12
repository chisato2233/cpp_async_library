#include"cst_async.h"
#include "cst_async.h"
#include <iostream>
#include<ranges>
#include"signal.h"

using namespace cst;











delegate<void()> tees;
// 定义一个协程函数，每帧加载协程
auto loadFrame() -> async::co_task<> {
    while (true) {
        co_await tees;
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
    using namespace async;

    std::cout << "dasd";
    auto i = suspend{};
   auto a = i || wait_next_frame{} || wait_time{1.0};
    

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
    test_signal();
    // 运行时更新循环，直到所有协程完成
    while (runtime->task_count() != 0) {
        
    	runtime->update(); // 更新运行时状态，推进协程执行
    }

    return 0;
}