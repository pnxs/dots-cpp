#pragma once
#include <dots/serialization/StringSerializer.h>

namespace dots::type
{
    namespace details
    {
        template <typename T>
        constexpr bool is_gtest_printable_v = std::disjunction_v<
            std::is_same<std::decay_t<T>, uuid_t>,
            std::is_same<std::decay_t<T>, timepoint_t>,
            std::is_same<std::decay_t<T>, steady_timepoint_t>,
            std::is_same<std::decay_t<T>, duration_t>,
            is_property<std::decay_t<T>>,
            is_enum<std::decay_t<T>>,
            std::is_base_of<Struct, std::decay_t<T>>
        >;
    }

    template <typename T, std::enable_if_t<details::is_gtest_printable_v<T>, int> = 0>
    std::ostream& operator << (std::ostream& os, T&& value)
    {
        return os << to_string(std::forward<T>(value));
    }
}

namespace dots::types
{
    template <typename T, std::enable_if_t<type::is_enum_v<std::decay_t<T>>, int> = 0>
    std::ostream& operator << (std::ostream& os, T&& value)
    {
        return os << to_string(std::forward<T>(value));
    }
}