# `cst` �����ռ�

## ����

`cst` �����ռ��ṩ��һ����Ϊ `delegate` ��ģ���࣬���ڹ�����ָ�롢��������ȵ�ί�С�

## ��

### `delegate`

`delegate` ����һ��ģ���࣬���ڹ�����ָ�롢��������ȵ�ί�С�

#### ��Ա����

##### �۲���
- `begin() const noexcept`: ����ί���к����б����ʼ��������
- `end() const noexcept`: ����ί���к����б�Ľ�����������
- `empty() const noexcept`: ����ί���Ƿ�Ϊ�ա�
- `is_frozen() const noexcept`: ����ί���Ƿ񱻶��ᡣ
- `size() const noexcept`: ����ί���к����б�Ĵ�С��
- `on_call() noexcept -> delegate<void, Args...>&`: �����ڵ���ί��ʱ������һ��ί�еĺ�����

##### �޸���
- `push_back(const func_type& f)`: ������������ӵ�ί�е�ĩβ��
- `push_back(func_type&& f)`: ������������ӵ�ί�е�ĩβ��
- `push_front(const func_type& f)`: ������������ӵ�ί�еĿ�ͷ��
- `push_front(func_type&& f)`: ������������ӵ�ί�еĿ�ͷ��
- `insert(const_iterator it, func_type&& f)`: ��ָ��λ�ò��뺯������
- `emplace_back(func_type&& f)`: ���첢����һ����������ί�е�ĩβ��
- `emplace_front(func_type&& f)`: ���첢����һ����������ί�еĿ�ͷ��

- `erase(const_iterator it)`: �Ƴ�ָ��λ�õĺ�������
- `erase(const_iterator begin, const_iterator end)`: �Ƴ�ָ����Χ�ڵĺ�������
- `erase_all()`: �Ƴ����к�������
- `delay_erase(const_iterator it, unsigned call_count = 1)`: �ӳ��Ƴ�ָ��λ�õĺ�������

- `freeze()`: ����ί�С�
- `unfreeze()`: �������ί�С�

- `call_all(Args&&... args) -> typename delegate_result<Re>::type`: ����ί���е����к�������
- `operator()(Args&&... args)`: ���غ������������������ `call_all()`��

- `operator co_await()`: ���� `co_await` �����������Э�̵��á�
- `operator bool() const`: ��ί��ת��Ϊ����ֵ���ж�ί���Ƿ�Ϊ�ա�

#### ��Ա����

- `is_banned`: �Ƿ����ί�С�
- `enable_call_next`: �Ƿ��ڵ���ʱ������һ��ί�еĺ�����

- `func_list_`: �����б�
- `erase_queue_`: �ӳ��Ƴ����С�
- `call_count_`: ���ü�����
- `is_frozen_`: �Ƿ񱻶��ᡣ
- `next_delegate_`: ��һ��ί�С�

## �����ռ�

### `async`

`async` �����ռ�����첽������ص����ͺͺ�����

#### ��

- `_delegate_awaiter`: ���ڴ����첽ί�е��õĸ����ࡣ

