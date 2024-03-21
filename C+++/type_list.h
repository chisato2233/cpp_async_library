#pragma once
#include<concepts>
namespace cst {
	struct _type_pair_tag {};
	template<class T> concept type_pair_cpt = std::derived_from<T, _type_pair_tag>;

	template<class T, size_t Index>
	struct type_pair :_type_pair_tag {
		using type = T;
		static constexpr auto index = Index;
	};

	struct _type_list_tag {};
	template<class T> concept type_list_cpt = std::derived_from<T, _type_list_tag>;


	template<class Header, class... Args>
	struct type_list :_type_list_tag {
		using type = Header;
		using next = type_list<Args...>;

	};

	template<class Header>
	struct type_list<Header> {
		using type = Header;
		using next = nullptr_t;
	};

	template<type_list_cpt TypeList, size_t Index>
	struct type_list_getter {
		using res = typename type_list_getter<typename TypeList::next, Index - 1>::res;
	};

	template<type_list_cpt TypeList>
	struct type_list_getter <TypeList, 0> {
		using res = typename TypeList::type;
	};

	template<size_t Index>
	struct type_list_getter<nullptr_t, Index> {
		static_assert(Index == 0, "Type List Index out of range");
	};


}