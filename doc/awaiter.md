# `suspend` �ṹ��

## ����

`suspend` �ṹ��������Э����ͣ�Ŀɵȴ��壬����ʵ�ֶ�̬��ͣ���ܡ�

### ��Ա����

- `bool await_resume() const noexcept`: ��ȡ��ͣ״̬��
- `bool await_ready() const`: �ж��Ƿ�����������
- `void await_suspend(std::coroutine_handle<P> caller) const`: ����ǰЭ�̡�

## ��Ա����

- `bool is_suspend`: ��ʾ�Ƿ���ͣ��

# `_delegate_awaiter` �ṹ��

## ����

`_delegate_awaiter` �ṹ��������ί�ж���Ŀɵȴ��壬���ڵȴ�ί�ж���Ϊ��ʱ����ִ�С�

### ��Ա����

- `static void await_resume()`: �ȴ�����ľ�̬������
- `bool await_ready() const`: �ж�ί�ж����Ƿ�Ϊ�ա�
- `void await_suspend(std::coroutine_handle<P> caller)`: ����ǰЭ�̣���ע��ί�ж���ĵ��á�

## ��Ա����

- `_delegate_impl<Re, Args...>* delegate_`: ί�ж����ָ�롣

# `wait_until` �ṹ��

## ����

`wait_until` �ṹ�������ڵȴ�ֱ�����������Ŀɵȴ��壬ÿ֡���һ��������

### ��Ա����

- `static void await_resume()`: �ȴ�����ľ�̬������
- `bool await_ready() const`: �ж������Ƿ������㡣
- `void await_suspend(std::coroutine_handle<P> caller)`: ����ǰЭ�̣���ע�������ļ�顣

## ��Ա����

- `std::function<bool()> predicate`: ����ν�ʺ�����

# `wait_while` �ṹ��

## ����

`wait_while` �ṹ�������ڵȴ�������������Ŀɵȴ��壬ÿ֡���һ��������

### ��Ա����

- `bool await_ready() const`: �ж������Ƿ������㡣
- `void await_resume()`: �ȴ�����ľ�̬������
- `void await_suspend(std::coroutine_handle<P> caller)`: ����ǰЭ�̣���ע�������ļ�顣

## ��Ա����

- `std::function<bool()> predicate`: ����ν�ʺ�����

# `wait_time` �ṹ��

## ����

`wait_time` �ṹ�������ڵȴ�һ��ʱ��Ŀɵȴ��塣

### ��Ա����

- `auto await_resume() -> std::chrono::duration<Rep, Period>`: ��ȡ�ȴ���ʱ�䡣
- `bool await_ready() const`: �ж��Ƿ�����������
- `void await_suspend(std::coroutine_handle<P> caller)`: ����ǰЭ�̣���ע�ᶨʱ����

## ��Ա����

- `std::chrono::duration<Rep, Period> time`: �ȴ���ʱ�䡣

# `wait_next_frame` �ṹ��

## ����

`wait_next_frame` �ṹ�������ڵȴ���һ֡�Ŀɵȴ��塣

### ���캯��

- `wait_next_frame(unsigned count = 1)`: ���캯����ָ���ȴ���֡����Ĭ��Ϊ 1��

### ��Ա����

- `unsigned await_resume() const`: ��ȡ�ȴ���֡����
- `bool await_ready() const`: �ж��Ƿ�����������
- `void await_suspend(std::coroutine_handle<P> caller)`: ����ǰЭ�̣��ȴ���һ֡��

## ��Ա����

- `unsigned count_`: �ȴ���֡����

