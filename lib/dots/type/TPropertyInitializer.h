#pragma once
#include <utility>

namespace dots::type
{
	template <typename P>
	struct TPropertyInitializer
	{
		using property_t = P;
		using value_t = typename P::value_t;
		template <typename... Args>
		TPropertyInitializer(Args&&... args) : value(std::forward<Args>(args)...) {}
		value_t value;
	};
}