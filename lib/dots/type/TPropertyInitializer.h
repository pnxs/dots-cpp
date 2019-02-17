#pragma once
#include <utility>
#include <type_traits>

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

	template <typename T>
	struct is_t_property_initializer : std::false_type {};

	template <typename P>
	struct is_t_property_initializer<TPropertyInitializer<P>> : std::true_type {};

	template <typename T>
	using is_t_property_initializer_t = typename is_t_property_initializer<T>::type;

	template <typename T>
	constexpr bool is_t_property_initializer_v = is_t_property_initializer_t<T>::value;
}