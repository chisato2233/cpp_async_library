#pragma once
#include"macro.h"
#include<array>
#include<any>
#include<tuple>
namespace cst {
	
	template<class... Args>
	struct type_map {
		template<class T>
		static constexpr auto get() -> size_t {
			return _get_impl<T,sizeof...(Args), Args...>();
		}

	private:
		template<class T, size_t TotalSize, class Header,class... Res>
		static constexpr auto _get_impl() -> size_t {

			if constexpr (std::is_same_v<T, Header>)
				return TotalSize - sizeof...(Res) - 1;
			
			else if (sizeof...(Res) == 0) return -1;
			else return _get_impl<T, TotalSize, Res...>();
		}

	};

	template<class... Args>
	struct tuple_dynamic {
		using _tuple_type_map = type_map<Args...>;

		tuple_dynamic(Args&&... args) :
			arr_{
				std::any{std::forward<Args>(args)}...
			}
		{}

		template<class T>
		constexpr auto get_index() { return _tuple_type_map::template get<T>(); }

		template<class T>
		auto& get(size_t i) { return std::any_cast<T&>(arr_[i]); }

		auto& get(size_t i) { return arr_[i]; }

		template<class T>
		auto& get() { return get<T>(get_index<T>()); }

	private:
		std::array<std::any, sizeof...(Args)> arr_;
	};

	template<class... Args>
	tuple_dynamic(Args...)->tuple_dynamic<Args...>;




	inline void test_tuple_dynamic() {
		tuple_dynamic t = { 1,2.0,"sdfdsfds" };
		t.get<int>() += 1;
		std::tuple<int, double, std::string> t2 = { 1,2.0,"sdfdsfds" };
		auto a = t.get(1);
		
		std::cout <<"hdsjakfahkjdshafkjdashf"<< t.get<double>() <<"\n\n\n";
	}
}