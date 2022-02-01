#pragma once
#include <utility>
#include <type_traits>

namespace dots::type
{
    template <typename P>
    struct PropertyInitializer
    {
        using property_t = P;
        using value_t = typename P::value_t;

        template <typename V = value_t, std::enable_if_t<std::is_arithmetic_v<V>, int> = 0>
        PropertyInitializer(value_t value) : value{ std::move(value) } {}
        template <typename... Args, typename V = value_t, std::enable_if_t<!std::is_arithmetic_v<V>, int> = 0>
        PropertyInitializer(Args&&... args) : value(std::forward<Args>(args)...) {}
        PropertyInitializer(const PropertyInitializer& other) = default;
        PropertyInitializer(PropertyInitializer&& other) = default;
        ~PropertyInitializer() = default;

        PropertyInitializer& operator = (const PropertyInitializer& rhs) = default;
        PropertyInitializer& operator = (PropertyInitializer&& rhs) = default;

        value_t value;
    };

    template <typename T>
    struct is_property_initializer : std::false_type {};

    template <typename P>
    struct is_property_initializer<PropertyInitializer<P>> : std::true_type {};

    template <typename T>
    using is_property_initializer_t = typename is_property_initializer<T>::type;

    template <typename T>
    constexpr bool is_property_initializer_v = is_property_initializer_t<T>::value;
}
