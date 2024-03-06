#include"cst_async.h"
#include "cst_async.h"
#include <iostream>
#include<ranges>

using namespace cst;






void test() {
    using namespace std;

}




delegate<void()> tees;
// ����һ��Э�̺�����ÿ֡����Э��
auto loadFrame() -> async::co_task<> {
    while (true) {
        co_await tees;
        std::cout << '>';
        co_await async::wait_next_frame{10000}; //�ȴ���������һ��֡
    }
}

// ����һ��Э�̺�����ÿ���ӡ���� 1��10
auto printNumbers() -> async::co_task<> {
    for (int i = 0; i <= 10; ++i) {
        co_await async::wait_time{1}; // �ȴ���һ��
        std::cout << i << '\n';

    }
    co_return; // ����Э��
}


//��Э��
auto mainCorotine(async::runtime* rt) -> async::co_task<> {
    // ��������֡Э��,Ȼ�󱣴�Э�̵�����
    auto& update = rt->start_task(loadFrame());
    // �ȴ���ӡ����Э�̽���
    co_await printNumbers();
    //�رռ���֡Э��
    rt->stop_task(update.get_ref());
    co_return;
}

int main() {
    // ��������ʱ����
    auto runtime = std::make_unique<async::runtime>();
    //������ʱ��
	timer::start();

    // ����Э��
    runtime->start_task(mainCorotine(runtime.get()));

    // ����ʱ����ѭ����ֱ������Э�����
    while (runtime->task_count() != 0) {
        
    	runtime->update(); // ��������ʱ״̬���ƽ�Э��ִ��
    }

    return 0;
}