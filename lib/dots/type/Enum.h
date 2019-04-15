#pragma once
#include <iostream>
#include "EnumDescriptor.h"

namespace dots::type
{
	class EnumDescriptor;

	template <typename T = void>
	struct Enum;

	template <>
	struct Enum<void>
	{
		using value_t = void;

		Enum(const EnumDescriptor& descriptor, int32_t value);
		Enum(const Enum& other) = default;
		Enum(Enum&& other) = default;
		~Enum() = default;

		Enum& operator = (const Enum& rhs) = default;
		Enum& operator = (Enum&& rhs) = default;

		const EnumDescriptor& descriptor() const;
		int32_t value() const;
		uint32_t tag() const;
		const std::string& identifier() const;

	private:

		const EnumDescriptor* _descriptor;
		int32_t _value;
	};

	template<typename T, typename = void>
	constexpr bool is_defined_v = false;
	template<typename T>
	constexpr bool is_defined_v<T, decltype(sizeof(T), void())> = true;
	template <typename T>
	struct has_enum_type : std::conditional_t<is_defined_v<Enum<T>>, std::true_type, std::false_type> {};
	template <typename T>
	using has_enum_type_t = typename has_enum_type<T>::type;
	template <typename T>
	constexpr bool has_enum_type_v = has_enum_type_t<T>::value;

	template <typename T>
	struct enum_type
	{
		static_assert(has_enum_type_v<T>, "T has to be a DOTS enum type");
		using type = Enum<T>;
	};
	template <typename T>
	using enum_type_t = typename enum_type<T>::type;
}

inline std::ostream& operator << (std::ostream& os, const dots::type::Enum<>& e)
{
	os << e.identifier();
	return os;
}

inline const std::string& to_string(const dots::type::Enum<>& e)
{
	return e.identifier();
}

template <typename T, std::enable_if_t<dots::type::has_enum_type_v<T>, int> = 0>
std::ostream& operator << (std::ostream& os, const T& enumerator)
{
	os << dots::type::enum_type_t<T>{ enumerator };
	return os;
}