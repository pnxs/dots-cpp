#pragma once
#include <dots/testing/gtest/gtest.h>

namespace dots::testing::details
{
    template <typename T>
    using is_expectation_t = std::is_constructible<::testing::Expectation, T>;

    template <typename T>
    constexpr bool is_expectation_v = is_expectation_t<T>::value;

    template <typename T>
    struct is_expectation_factory
    {
        static auto IsExpectationFactory()
        {
            if constexpr (std::is_invocable_v<T>)
            {
                return is_expectation_t<std::invoke_result_t<T>>{};
            }
            else
            {
                return std::false_type{};
            }
        }

        using type = decltype(IsExpectationFactory());
    };

    template <typename T>
    using is_expectation_factory_t = typename is_expectation_factory<T>::type;

    template <typename T>
    constexpr bool is_expectation_factory_v = is_expectation_factory_t<T>::value;

    template <typename T, typename = void>
    struct expectation_signature {};

    template <typename F>
    struct expectation_signature<::testing::internal::TypedExpectation<F>> { using type = F; };

    template <typename T>
    using expectation_signature_t = typename expectation_signature<T>::type;

    template <typename T, typename = void>
    struct is_compatible_action : std::false_type {};

    template <typename T, typename R, typename... Args>
    struct is_compatible_action<T, R(Args...)>
    {
        static auto IsCompatibleAction()
        {
            if constexpr (std::is_invocable_v<T>)
            {
                return std::is_same<std::invoke_result_t<T>, void>{};
            }
            else if constexpr (std::is_invocable_v<T, Args...>)
            {
                return std::is_same<std::invoke_result_t<T, Args...>, void>{};
            }
            else
            {
                return std::false_type{};
            }
        }

        using type = decltype(IsCompatibleAction());
    };

    template <typename T, typename Signature>
    using is_compatible_action_t = typename is_compatible_action<T, Signature>::type;

    template <typename T, typename Signature>
    constexpr bool is_compatible_action_v = is_compatible_action_t<T, Signature>::value;

    template <typename... Ts>
    using expectation_tuple_t = std::tuple<std::conditional_t<is_expectation_v<std::decay_t<Ts>>, std::decay_t<Ts>&, std::decay_t<Ts>>...>;
}
