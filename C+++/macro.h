#pragma once
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

namespace cst {
	template<class T> using opt  = std::optional<T>;
	template<class T> using ref  = std::reference_wrapper<T>;
	template<class T> using ptr  = std::shared_ptr<T>;
	template<class T> using uptr = std::unique_ptr<T>;

	template<class T> struct rptr {
		constexpr rptr() = default;
		constexpr rptr(std::nullptr_t) :weak_ptr_{} {}

		template<std::same_as<T> T2>
		rptr (const std::shared_ptr<T2>& sp) :weak_ptr_(sp) {}

		template<std::same_as<T> T2>
		rptr(const std::weak_ptr<T2>& wp) :weak_ptr_(wp) {}

		explicit operator bool() { return !weak_ptr_.expired(); }

		explicit operator ptr<T>() {
			if(weak_ptr_.expired())
				return nullptr;
			return weak_ptr_.lock();
		}

		auto operator*()->T& { return weak_ptr_.lock().operator*(); }
		auto operator->()->T* { return weak_ptr_.lock().operator->(); }

	private:
		std::weak_ptr<T> weak_ptr_;
	};
	template<class T> rptr(const ptr<T>) -> rptr<T>;

	inline void test() {
		struct _ { int i; };
		rptr<_> rp;
		{
			auto sp = std::make_shared<_>();
			rp = sp;
			if (rp) { std::cout << rp->i; }

		}
		if(rp) std::cout << rp->i;
	}

	struct no_copy {

		no_copy(no_copy&) = delete;
		no_copy(no_copy&&) = default;

		no_copy& operator=(no_copy&) = delete;
		no_copy& operator=(no_copy&&) = default;

	protected:
		no_copy() = default;
		~no_copy() = default;
	};

	struct no_move {


		no_move(no_move&) = default;
		no_move(no_move&&) = delete;

		no_move& operator=(no_move&) = default;
		no_move& operator=(no_move&&) = delete;
	protected:
		no_move() = default;
		~no_move() = default;
	};

	struct no_copy_move:no_copy, no_move {};
	using no_cpmv = no_copy_move;


	template<class Index,class... Source>
	struct unit {
		template<class... Args>
		unit(Index index, Args&&... args) :index{index},value(std::forward<Args>(args)...) {}

		template<class... Src>
		constexpr auto operator<=>(const unit<Index,Src...>& other)const {
			return index<=>other.index;
		}

		template<size_t Src_Index = 0>
		constexpr auto get() const noexcept{
			return std::get<Src_Index>(value);
		}

		

		Index index;
		std::tuple<Source...> value;
	};



}
