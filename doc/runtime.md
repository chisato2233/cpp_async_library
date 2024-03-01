# `runtime` ��

## ����

`runtime` ����һ��Э�̵����������ڹ���͵���Э�������ִ�С�

### ��Ա����

- `register_task(co_task<T>&& task) -> register_option`: ע��һ��Э�����񣬲�����ע��ѡ�
- `register_task(F f, Args&&... args)`: ע��һ��Э������ʹ�ø����ĺ�������Ͳ�����
- `cancel_task(co_task_base* task_ref)`: ȡ��һ��Э������
- `start_task(co_task<T>&& task) -> co_task<T>&`: ע�Ტ����һ��Э�����񣬲������������
- `start_task(uint64_t id) -> co_task_base*`: �������� ID ע�Ტ����һ��Э������
- `stop_task(co_task_base* task)`: ֹͣһ��Э������
- `stop_task(uint64_t id)`: �������� ID ֹͣһ��Э������
- `suspend_task(co_task_base* task)`: ��ͣһ��Э������
- `suspend_task(uint64_t id)`: �������� ID ��ͣһ��Э������
- `resume_task(co_task_base* task)`: �ָ�һ����ͣ��Э������
- `resume_task(uint64_t id)`: �������� ID �ָ�һ����ͣ��Э������
- `update()`: ����Э������״̬��
- `task_count() const noexcept -> std::size_t`: ���ص�ǰ�����Э������������

## �ṹ�� `runtime::register_option`

### ����

`register_option` �ṹ����ע��ѡ�����ͣ�����ע��Э������ʱ�ṩһЩ����ѡ�

### ��Ա����

- `add_predicate(const std::function<bool()>& predicate) &&`: ���һ��ν�ʺ�������ν�ʷ��� true ʱ����ע�������
- `add_timer(std::chrono::duration<Rep,Period> duration) &&`: ���һ����ʱ����ָ��ʱ�������ע�������

## ��������

- `_builtin_call_runtime_start_task(runtime* rt, co_task<T>&& task)`: ���ú��������ڵ��� `runtime` ��� `start_task` ������
- `_builtin_call_runtime_stop_task(runtime* rt, co_task_base* task)`: ���ú��������ڵ��� `runtime` ��� `cancel_task` ������
- `_builtin_call_runtime_suspend_task(runtime* rt, co_task_base* task)`: ���ú��������ڵ��� `runtime` ��� `suspend_task` ������
- `_builtin_call_runtime_resume_task(runtime* rt, co_task_base* task)`: ���ú��������ڵ��� `runtime` ��� `resume_task` ������

