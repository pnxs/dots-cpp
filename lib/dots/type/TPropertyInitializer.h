#pragma once
#include <utility>
#include <type_traits>

namespace dots::type
{
	// note: the additional template parameter is necessary to circumvent an odd and unintuitive definition in the C++ ISO standard.
	// when a user defines a property with the name 'value', the code generator will make a property type with the name 'value_t' and
	// even though the usage of the 'typename' keyword should specify to the compiler that the identifier refers to the alias in the
	// property base, it actually refers to the constructor. this is correctly remarked by Clang and just fails up to at least GCC 8.
	template <typename P, typename V = typename P::value_t>
	struct TPropertyInitializer
	{
		using property_t = P;
		using value_t = V;

		template <typename... Args>
		TPropertyInitializer(Args&&... args) : value(std::forward<Args>(args)...) {}
		TPropertyInitializer(const TPropertyInitializer& other) = default;
		TPropertyInitializer(TPropertyInitializer&& other) = default;
		~TPropertyInitializer() = default;

		TPropertyInitializer& operator = (const TPropertyInitializer& rhs) = default;
		TPropertyInitializer& operator = (TPropertyInitializer&& rhs) = default;

		value_t value;
	};

	template <typename T>
	struct is_t_property_initializer : std::false_type {};

	template <typename P, typename V>
	struct is_t_property_initializer<TPropertyInitializer<P, V>> : std::true_type {};

	template <typename T>
	using is_t_property_initializer_t = typename is_t_property_initializer<T>::type;

	template <typename T>
	constexpr bool is_t_property_initializer_v = is_t_property_initializer_t<T>::value;
}