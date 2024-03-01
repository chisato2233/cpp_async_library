# `co_task` ��

## ����

`co_task` ��һ��Э�̶���ģ���࣬���ڹ���Э�������ִ�С�

### ��Ա����

- `co_handle`: ��ʾЭ�̾�������ͣ��� `std::coroutine_handle` �ı�����

### ��Ա����

- `co_task(co_handle handle)`: ���캯����ʹ�ø�����Э�̾������һ��Э���������
- `~co_task()`: ��������������Э���������
- `operator()()`: ������������أ�ִ��Э������
- `done() -> bool`: ����Э�������Ƿ�����ɡ�
- `destroy()`: ����Э����������
- `get_coroutine_handle() const noexcept -> co_handle`: ����Э�̾����
- `get_id() const noexcept -> std::uint64_t`: ����Э�������Ψһ��ʶ����
- `get_runtime() const noexcept -> runtime*`: ����Э����������������ʱ������
- `state() const noexcept -> task_state&`: ����Э�������״̬��
- `on_start() const noexcept -> delegate<task_promise_base, void()>`: ����Э������ʼʱ��ί�С�
- `on_resume() const noexcept -> delegate<task_promise_base, void()>`: ����Э������ָ�ִ��ʱ��ί�С�
- `on_done() const noexcept -> delegate<task_promise_base, void()>`: ����Э���������ʱ��ί�С�

### ��Ա���� operator co_await

- `operator co_await()`: ����Э�̵ȴ�����������غ�����

## �ṹ�� `co_task::promise_type`

### ����

`promise_type` �� `co_task` ��Э�� Promise ���ͣ����ڹ���Э�̵�״̬��ִ�й��̡�

### ��Ա����

- `get_return_object() noexcept -> co_task`: ��ȡЭ���������
- `initial_suspend() noexcept -> std::suspend_always`: ����Э�̵ĳ�ʼ��ͣ�㡣
- `final_suspend() noexcept -> std::suspend_never`: ����Э�̵�������ͣ�㡣
- `unhandled_exception()`: ����δ������쳣��
- `promise_transform(std::suspend_always)`: ת�� Promise ����״̬��
