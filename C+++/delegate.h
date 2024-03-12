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
	struct _delegate_impl : no_copy  {
		_delegate_impl() = default;

		using func_type= std::function<Re(Args...)>;
		/*auto begin()const noexcept { return func_map_.begin(); }
		auto end()const noexcept { return func_map_.end(); }*/

		

		bool empty()const noexcept { return func_map_.empty(); }
		bool is_frozen()const noexcept { return is_frozen_; }
		size_t size()const noexcept { return func_map_.size(); }

		auto on_call() noexcept->_delegate_impl<void,Args...>& {
			enable_call_next = true;
			if(!next_delegate_)
				next_delegate_ = std::make_unique<_delegate_impl<void, Args...>>();
			return *next_delegate_;
		}

		auto add(func_type&& f) {
			func_map_.insert({ ++next_func_id_, std::move(f) });
			return next_func_id_ - 1;
		}


		auto remove(uint64_t id) { func_map_.erase(id); }

		auto delay_remove(uint64_t id,int count = 1) {
			erase_queue_.emplace(count + call_count_,id );
		}

		auto clear() { func_map_.clear(); }

		void freeze() { is_frozen_ = true; }
		void unfreeze() { is_frozen_ = false; }

		auto call_all(Args&&... args) -> typename delegate_result<Re>::type {
			if(!is_frozen_) ++call_count_;

			if(is_banned||empty()||is_frozen_) 
				return delegate_result<Re>::null();

			_update_erase_queue();
			return _call_next(std::forward<Args>(args)...),_for_each_call(std::forward<Args>(args)...);
		}


		auto& operator +=(func_type&& f) {
			add(std::move(f));
			return *this;
		}

		auto& operator -=(uint64_t id) {
			remove(id);
			return *this;
		}

		auto operator()(Args... args) {
			return call_all(std::forward<Args>(args)...);
		}

		auto operator co_await() {
			return async::_delegate_awaiter<Re, Args...>{this};
		}


	private:
		auto _for_each_call(Args&&... args) -> typename delegate_result<Re>::type {
			if constexpr (std::is_same_v<Re, void>) {
				for (auto& [_,f] : func_map_)
					std::invoke(f, std::forward<Args>(args)...);
				return;
			}
			else {
				std::vector<opt<Re>> res;
				for (auto& [_,f] : func_map_)
					res.emplace_back(opt<Re>{
					std::invoke(f, std::forward<Args>(args)...)
				});
				return res;
			}
			
		}

		auto _update_erase_queue() {
			while(!erase_queue_.empty() && call_count_<= erase_queue_.top().index) {
				func_map_.erase(erase_queue_.top().get<0>());
				erase_queue_.pop();
			}
		}

		auto _call_next(Args&&... args) {
			if(next_delegate_ && enable_call_next) {
				next_delegate_->call_all(std::forward<Args>(args)...);
				if (next_delegate_->empty())
					enable_call_next = false, next_delegate_ = nullptr;
			}
		}

	public:
		bool is_banned = false;
		bool enable_call_next = false;
	private:
		
		std::map<uint64_t,func_type> func_map_;

		std::priority_queue <
			unit<unsigned, uint64_t>,
			std::vector<unit<unsigned, uint64_t>>,
			std::greater<>
		> erase_queue_;

		uint64_t next_func_id_ = 0;


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
		delegate() = default;
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
	template<class Obj, class Fx> delegate(Obj*) -> delegate<Obj, Fx>;
}
