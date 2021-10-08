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

    template <typename ArgHead, typename... ArgTail>
    auto& expectation_sequence_recursive(const ::testing::Sequence& sequence, ArgHead&& argHead, ArgTail&&... argTail)
    {
        auto& expectation = [](auto&& arg) -> auto&
        {
            using arg_t = decltype(arg);

            constexpr bool IsExpectation = is_expectation_v<arg_t>;
            constexpr bool IsExpectationFactory = is_expectation_factory_v<arg_t>;

            static_assert(IsExpectation || IsExpectationFactory, "expectation sequence argument must be either an expectation or a trivially invocable expectation factory");

            if constexpr (IsExpectation)
            {
                return std::forward<arg_t>(arg);
            }
            else if constexpr (IsExpectationFactory)
            {
                return std::invoke(std::forward<arg_t>(arg));
            }
            else
            {
                return EXPECT_CALL(std::declval<::testing::MockFunction<void()>>(), Call());
            }
        }(std::forward<decltype(argHead)>(argHead)).InSequence(sequence);

        using expectation_t = std::decay_t<decltype(expectation)>;

        if constexpr (sizeof...(argTail) > 0)
        {
            auto argTailRefs = std::forward_as_tuple(std::forward<decltype(argTail)>(argTail)...);
            using arg_tail_head_t = std::tuple_element_t<0, decltype(argTailRefs)>;

            if constexpr (is_compatible_action_v<arg_tail_head_t, expectation_signature_t<expectation_t>>)
            {
                expectation.WillOnce(::testing::DoAll(std::forward<arg_tail_head_t>(std::get<0>(argTailRefs))));

                if constexpr (sizeof...(argTail) > 1)
                {
                    return std::apply([&sequence](auto&&/* argTailHead*/, auto&&... argTailTail) -> auto&
                    {
                        return expectation_sequence_recursive(sequence, std::forward<decltype(argTailTail)>(argTailTail)...);
                    }, argTailRefs);
                }
                else
                {
                    return expectation;
                }
            }
            else
            {
                (void)expectation;
                return expectation_sequence_recursive(sequence, std::forward<decltype(argTail)>(argTail)...);
            }
        }
        else
        {
            return expectation;
        }
    }

    template <typename ArgHead, typename... ArgTail>
    auto& expectation_sequence(const ::testing::Sequence& sequence, ArgHead&& argHead, ArgTail&&... argTail)
    {
        if constexpr (std::is_invocable_v<decltype(argHead)>)
        {
            auto& expectation = expectation_sequence_recursive(sequence, std::forward<decltype(argTail)>(argTail)...);
            std::invoke(std::forward<decltype(argHead)>(argHead));

            return expectation;
        }
        else
        {
            return expectation_sequence_recursive(sequence, std::forward<decltype(argHead)>(argHead), std::forward<decltype(argTail)>(argTail)...);
        }
    }
}

#define DOTS_NAMED_EXPECTATION_SEQUENCE                                                                   \
[&](const ::testing::Sequence& sequence, auto&&... args) -> auto&                                         \
{                                                                                                         \
    return dots::testing::details::expectation_sequence(sequence, std::forward<decltype(args)>(args)...); \
}  

#define DOTS_EXPECTATION_SEQUENCE                                                                         \
[&](auto&&... args) -> auto&                                                                              \
{                                                                                                         \
    static ::testing::Sequence Sequence;                                                                  \
    return dots::testing::details::expectation_sequence(Sequence, std::forward<decltype(args)>(args)...); \
}
