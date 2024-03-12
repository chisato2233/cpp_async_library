#pragma once
#include"delegate.h"
#include<functional>


namespace cst {
	struct TT {
		int i;
	};

	template<class T> concept number = std::integral<T> || std::floating_point<T>;
	template<number Nm = int>
	struct property_num {
		property_num() = default;
		property_num(Nm value) : value_(value) {}
		operator Nm() { return value_; }

		Nm& value()const noexcept { return value_; }
		auto& on_change()const noexcept { return *on_change_; }

		void change(const Nm& other) {
			if (on_change().empty()) return;

			Nm old = value_;
			value_ = other;
			on_change()(std::move(old), other);
		}

		property_num& operator=(const Nm& other) {
			change(other);
			return *this;
		}

		auto operator<=>(const Nm& other) const { return value_ <=> other; }

		auto operator+=	(const Nm& other) { change(value_ + other); return *this; }
		auto operator-=	(const Nm& other) { change(value_ - other); return *this; }
		auto operator*=	(const Nm& other) { change(value_ * other); return *this; }
		auto operator/=	(const Nm& other) { change(value_ / other); return *this; }
		auto operator%=	(const Nm& other) { change(value_ % other); return *this; }
		auto operator&=	(const Nm& other) { change(value_ & other); return *this; }
		auto operator|=	(const Nm& other) { change(value_ | other); return *this; }
		auto operator^=	(const Nm& other) { change(value_ ^ other); return *this; }
		auto operator<<=(const Nm& other) { change(value_ << other); return *this; }
		auto operator>>=(const Nm& other) { change(value_ >> other); return *this; }

		auto operator++() { change(value_ + 1); return *this; }
		auto operator--() { change(value_ - 1); return *this; }
		auto operator++(int) { auto old = *this; change(value_ + 1); return old; }
		auto operator--(int) { auto old = *this; change(value_ - 1); return old; }


		//int get() const { return value_; }

		
	private:
		Nm value_{};
		ptr<delegate<void(Nm,Nm)>> on_change_ = std::make_shared<delegate<void(Nm,Nm)>>();
	};

	template<number Nm>property_num(Nm) -> property_num<Nm>;

	template<class... Args>
	struct signal {
		signal() = default;

		signal(ptr<delegate<void(Args...)>> d):
			my_delegate_(d) {}

		void bind(ptr<delegate<void(Args...)>> p) { my_delegate_ = p; }
		void create_delegate() { my_delegate_ = std::make_shared<delegate<void(Args...)>>(); }


		auto get_delegate()  -> delegate<void(Args...)>& {
			if (!my_delegate_) create_delegate();
			return *my_delegate_;
		}

		auto operator()() -> delegate<void(Args...)>& { return get_delegate(); }

		void notify(Args... args)  {
			if ( my_delegate_->empty()) 
				return;
			(*my_delegate_)(std::forward<Args>(args)...);
		}

	private:
		ptr<delegate<void(Args...)>> my_delegate_;
	};

	template<class T>
	struct property {
		//property() = default;

		T get() { return getter(); }
		void set(const T& new_val) {
			T old = value;
			setter(new_val);
			on_change.notify(old, value);
		}

		operator const T()const {
			std::cout << "value!!";
			return value;
		}

		operator T&  () { return value; }
	public:
		T value;
		std::function<T()> getter{ [this] {return value; } };
		std::function<void(const T&)> setter{ [this](const T& new_val) {return value = new_val; } };
		signal<T, const T&> on_change { std::make_shared<delegate<void(T,const T&)>>() };
	};

	template<class T> property(T)-> property<T>;


	inline void test_signal(){
		property_num s{ 0.0 };
		s.on_change() += [](int old, int new_) { std::cout << "old: " << old << " new: " << new_ << std::endl; };
		s += 1.2;
		property<int> p1;

		property<int> p2 {
			.value = 4,
			.getter = [self = &p2] {return self->value * self->value; },
			.setter = [self = &p2](const int& new_val) {return self->value = 114514; }
		};

		p2.on_change() += [](int old, int new_) { std::cout << "old: " << old << " new: " << new_ << std::endl; };
		p2 += 2;

		std::cout << p2;
	}
}




