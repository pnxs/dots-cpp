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
    struct PropertyInitializer
    {
        using property_t = P;
        using value_t = V;

        template <typename V_ = V, std::enable_if_t<std::is_arithmetic_v<V_>, int> = 0>
        PropertyInitializer(V v) : value{ std::move(v) } {}
        template <typename... Args, typename V_ = V, std::enable_if_t<!std::is_arithmetic_v<V_>, int> = 0>
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

    template <typename P, typename V>
    struct is_property_initializer<PropertyInitializer<P, V>> : std::true_type {};

    template <typename T>
    using is_property_initializer_t = typename is_property_initializer<T>::type;

    template <typename T>
    constexpr bool is_property_initializer_v = is_property_initializer_t<T>::value;
}