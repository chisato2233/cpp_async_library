#pragma once
#include<iostream>
#include<list>
#include<queue>
#include<map>
#include<type_traits>
#include"macro.h"
#include<functional>


namespace cst {
	template<class...> inline constexpr bool always_false = false;

	template<class... A> struct delegate;

	template<class Re> struct delegate_result {
		using type = std::vector<opt<Re>>;
		using element_type = opt<Re>;
		static type null(){ return {}; }
	};

	template<> struct delegate_result<void> {
		using type = void;
		using element_type = void;
		static type null(){}
	};
	namespace async {
		template<class Re,class...Args> struct _delegate_awaiter;
	}

	template<class Re, class... Args>
	struct _delegate_impl  {
		using func_type= std::function<Re(Args...)>;
		using iterator = typename std::list<func_type>::iterator;
		using const_iterator = typename std::list<func_type>::const_iterator;

		// 
		auto begin()const noexcept { return func_list_.begin(); }
		auto end()const noexcept { return func_list_.end(); }
		void empty()const noexcept { return func_list_.empty(); }
		bool is_frozen()const noexcept { return is_frozen_; }
		size_t size()const noexcept { return func_list_.size(); }

		auto on_call() noexcept->_delegate_impl<void,Args...>& {
			enable_call_next = true;
			if(!next_delegate_)
				next_delegate_ = std::make_unique<_delegate_impl<void, Args...>>();
			return *next_delegate_;
		}


		void push_back(const func_type& f) { func_list_.push_back(f); }
		void push_back(func_type&& f) { func_list_.push_back(std::move(f)); }

		void push_front(const func_type& f) { func_list_.push_front(f); }
		void push_front(func_type&& f) { func_list_.push_front(std::move(f)); }

		void insert(const_iterator it, func_type&& f) { func_list_.insert(it, f); }

		auto emplace_back(func_type&& f) { return func_list_.emplace_back(std::move(f)); }
		auto emplace_front(func_type&& f){return func_list_.emplace_front(std::move(f));}

		auto erase(const_iterator it) { return func_list_.erase(it); }
		auto erase(const_iterator begin, const_iterator end) { return func_list_.erase(begin, end); }
		auto erase_all() { erase(begin(), end()); }
		void delay_erase(const_iterator it, unsigned call_count = 1) {
			erase_queue_.push({call_count,  it });
		}

		void freeze() { is_frozen_ = true; }
		void unfreeze() { is_frozen_ = false; }

		auto call_all(Args&&... args) -> typename delegate_result<Re>::type {
			if(!is_frozen_) ++call_count_;

			if(is_banned||empty()||is_frozen_) 
				return delegate_result<Re>::null();

			_update_erase_queue();
			return _for_each_call(std::forward<Args>(args)...);
		}



		
		auto operator()(Args&&... args) {
			return call_all(std::forward<Args>(args)...);
		}

		auto operator co_await() {
			return async::_delegate_awaiter<Re, Args...>{this};
		}


	private:
		auto _for_each_call(Args&&... args) -> typename delegate_result<Re>::type {
			if constexpr (std::is_same_v<Re, void>) {
				for (auto i : *this)
					std::invoke(i, std::forward<Args>(args)...);
				return;
			}
			else {
				std::vector<opt<Re>> res;
				for (auto i : *this)
					res.emplace_back(opt<Re>{
					std::invoke(i, std::forward<Args>(args)...)
				});
				return res;
			}
		}

		auto _update_erase_queue() {
			while(!erase_queue_.empty() && call_count_ >= erase_queue_.top()) {
				erase(erase_queue_.top());
				erase_queue_.pop();
			}
		}

		auto _call_next() {
			if(next_delegate_ && enable_call_next) {
				next_delegate_->call_all();
			}
		}

	public:
		bool is_banned = false;
		bool enable_call_next = false;
	private:
		
		std::list<func_type> func_list_;


		std::priority_queue<
			unit<unsigned,iterator>,
			std::vector<unit<unsigned,iterator>>,
			std::greater<>
		> erase_queue_;

		unsigned int call_count_ = 0;
		bool is_frozen_ = false;
		uptr<_delegate_impl<void, Args...>> next_delegate_ = nullptr;
	};





	template<class... T> struct _get_delegate_impl {
		static_assert(always_false<T...>, "this is not a function type");
	};



	template<class Re, class... Args>
	struct _get_delegate_impl<Re(Args...)> {
		using type = _delegate_impl<Re, Args...>;
	};

	template<class Obj, class Re, class... Args>
	struct _get_delegate_impl<Obj, Re(Args...)> {
		using type = struct _ :_delegate_impl<Re, Obj*, Args...> {
			auto operator()(Args&&... args) {
				std::invoke(
					&_delegate_impl<Re, Obj*, Args...>::operator(),
					this,
					std::move(static_cast<delegate<Obj, Re(Args...)>*>(this)->obj_),
					std::forward<Args>(args)...
				);
			}
		};
	};




	template<class Fx>
	struct delegate<Fx> :_get_delegate_impl<Fx>::type {
		explicit operator bool() const { return !this->empty(); }

	};


	template<class Obj, class Fx>
	struct delegate<Obj, Fx> :_get_delegate_impl<Obj, Fx>::type {
		friend struct _get_delegate_impl<Obj, Fx>;

		delegate(Obj* obj) : obj_{ obj } {}

		void bind(Obj* obj) { obj_ = obj; }
		auto get() const->Obj* { return obj_; }

		explicit operator bool()const { return obj_ && !this->empty(); }
	private:
		Obj* obj_ = nullptr;
	};

	delegate()->delegate<void()>;

}
