#pragma once
#include <memory>
#include <optional>

namespace cst {
	template<class T> using opt  = std::optional<T>;
	template<class T> using ref  = std::reference_wrapper<T>;
	template<class T> using ptr  = std::shared_ptr<T>;
	template<class T> using uptr = std::unique_ptr<T>;

	template<class Index,class... Source>
	struct unit {
		template<class... Args>
		unit(Index index, Args&&... args) :index{index},value(std::forward<Args>(args)...) {}

		template<class... Src>
		auto operator<=>(const unit<Index,Src...>& other)const {
			return index<=>other.index;
		}

		Index index;
		std::tuple<Source...> value;
	};
	

}
