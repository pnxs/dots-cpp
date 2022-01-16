#pragma once
#include <type_traits>

namespace dots::tools
{
    template <typename T>
    using always_false = std::false_type;

    template <typename T>
    using always_false_t = always_false<T>;

    template <typename T>
    constexpr bool always_false_v = always_false<T>::value;
}
