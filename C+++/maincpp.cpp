#include"cst_async.h"

using namespace cst;
delegate<void(int)> te;
int cnt;

auto test(int i)->async::co_task<> {
	
	std::cout << i << '\n';
	cnt++;
	if(i<=1) 
		co_await test(i + 1);
	co_return;
}

void test_coro() {
	timer::start();
	auto runtime = std::make_unique<async::runtime>();

	runtime->start_task(test(0)).on_done().push_back([](auto&&...) {std::cout << "done"; });

	while (cnt<=100 || runtime->task_count()!=0) {
		timer::update();
		runtime->update();
	}
}

struct thread_test {
	void test(int i) {
		;
		thread_test t{};
		std::cout << i << '\n';
		if(i<=10000) {
			std::thread(&thread_test::test, &t,i+1).join();
		}
		
	}

};


void test_thread() {
	std::condition_variable on_exit;
	std::mutex mm;
	std::unique_lock l{ mm };

	thread_test t{};
	std::thread(&thread_test::test, &t, 0).join();
}

int main() {
	
	test_coro();
	std::cout << timer::now();
}


