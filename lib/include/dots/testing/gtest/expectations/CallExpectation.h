#pragma once
#include <dots/testing/gtest/gtest.h>

#if defined(DOTS_ENABLE_DEPRECATED_SEQUENCE_SUPPORT)

namespace dots::testing::deprecated::details
{
    template <typename T, typename = void>
    struct is_expectation : std::false_type {};

    template <typename T>
    struct is_expectation<T, std::void_t<decltype(std::declval<T>().WillOnce(::testing::DoAll()))>> : std::true_type {};

    template <typename T>
    using is_expectation_t = typename is_expectation<T>::type;

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

    template <typename DefaultExpectationFactory, typename ArgHead, typename... ArgTail>
    auto& expect_consecutive_call_sequence_recursive(DefaultExpectationFactory defaultExpectationFactory, ArgHead&& argHead, ArgTail&&... argTail)
    {
        auto& expectation = [&defaultExpectationFactory](auto&& arg) -> auto&
        {
            using arg_t = decltype(arg);
            using decayed_arg_t = std::decay_t<arg_t>;

            if constexpr (is_expectation_v<decayed_arg_t>)
            {
                return std::forward<arg_t>(arg);
            }
            else if constexpr (is_expectation_factory_v<decayed_arg_t>)
            {
                return std::invoke(std::forward<arg_t>(arg));
            }
            else
            {
                return std::invoke(defaultExpectationFactory, std::forward<arg_t>(arg));
            }
        }(std::forward<decltype(argHead)>(argHead));

        using decayed_expectation_t = std::decay_t<decltype(expectation)>;

        if constexpr (sizeof...(argTail) > 0)
        {
            auto argTailTuple = std::make_tuple(std::forward<decltype(argTail)>(argTail)...);
            using arg_tail_head_t = std::tuple_element_t<0, decltype(argTailTuple)>;

            if constexpr (is_compatible_action_v<arg_tail_head_t, expectation_signature_t<decayed_expectation_t>>)
            {
                auto action = std::forward<arg_tail_head_t>(std::get<0>(argTailTuple));
                return expectation.WillOnce(::testing::DoAll(
                    [defaultExpectationFactory{ std::move(defaultExpectationFactory) }, argTailTuple{ std::move(argTailTuple) }](auto&&...)
                    {
                        if constexpr (sizeof...(argTail) > 1)
                        {
                            std::apply([defaultExpectationFactory{ std::move(defaultExpectationFactory) }](auto&&/* argTailHead*/, auto&&... argTailTail) -> auto&
                            {
                                return expect_consecutive_call_sequence_recursive(defaultExpectationFactory, std::forward<decltype(argTailTail)>(argTailTail)...);
                            }, argTailTuple);
                        }
                    },
                    std::move(action)
                ));
            }
            else
            {
                return expectation.WillOnce(::testing::DoAll([defaultExpectationFactory{ std::move(defaultExpectationFactory) }, argTailTuple{ std::move(argTailTuple) }](auto&&...)
                {
                    std::apply([defaultExpectationFactory{ std::move(defaultExpectationFactory) }](auto&&... argTailTail) -> auto&
                    {
                        return expect_consecutive_call_sequence_recursive(defaultExpectationFactory, std::forward<decltype(argTailTail)>(argTailTail)...);
                    }, argTailTuple);
                }));
            }
        }
        else
        {
            return expectation;
        }
    }

    template <typename DefaultExpectationFactory, typename ArgHead, typename... ArgTail>
    auto& expect_named_call_sequence_recursive(DefaultExpectationFactory& defaultExpectationFactory, const ::testing::Sequence& sequence, ArgHead&& argHead, ArgTail&&... argTail)
    {
        auto& expectation = defaultExpectationFactory(std::forward<decltype(argHead)>(argHead)).InSequence(sequence);;
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
                    return std::apply([&defaultExpectationFactory, &sequence](auto&&/* argTailHead*/, auto&&... argTailTail) -> auto&
                    {
                        return expect_named_call_sequence_recursive(defaultExpectationFactory, sequence, std::forward<decltype(argTailTail)>(argTailTail)...);
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
                return expect_named_call_sequence_recursive(defaultExpectationFactory, sequence, std::forward<decltype(argTail)>(argTail)...);
            }
        }
        else
        {
            return expectation;
        }
    }


    template <typename DefaultExpectationFactory, typename ArgHead, typename... ArgTail>
    auto& expect_consecutive_call_sequence(DefaultExpectationFactory defaultExpectationFactory, ArgHead&& argHead, ArgTail&&... argTail)
    {
        if constexpr (std::is_invocable_v<decltype(argHead)>)
        {
            auto& expectation = expect_consecutive_call_sequence_recursive(defaultExpectationFactory, std::forward<decltype(argTail)>(argTail)...);
            std::invoke(std::forward<decltype(argHead)>(argHead));

            return expectation;
        }
        else
        {
            return expect_consecutive_call_sequence_recursive(defaultExpectationFactory, std::forward<decltype(argHead)>(argHead), std::forward<decltype(argTail)>(argTail)...);
        }
    }

    template <typename DefaultExpectationFactory, typename ArgHead, typename... ArgTail>
    auto& expect_named_call_sequence(DefaultExpectationFactory defaultExpectationFactory, const ::testing::Sequence& sequence, ArgHead&& argHead, ArgTail&&... argTail)
    {
        if constexpr (std::is_invocable_v<decltype(argHead)>)
        {
            auto& expectation = expect_named_call_sequence_recursive(defaultExpectationFactory, sequence, std::forward<decltype(argTail)>(argTail)...);
            std::invoke(std::forward<decltype(argHead)>(argHead));

            return expectation;
        }
        else
        {
            return expect_named_call_sequence_recursive(defaultExpectationFactory, sequence, std::forward<decltype(argHead)>(argHead), std::forward<decltype(argTail)>(argTail)...);
        }
    }
}

#define DOTS_EXPECT_CONSECUTIVE_CALL_SEQUENCE(defaultExpectationFactory_, ...)                                                                     \
[&](auto defaultExpectationFactory, auto&&... args) -> auto&                                                                                       \
{                                                                                                                                                  \
    return dots::testing::deprecated::details::expect_consecutive_call_sequence(defaultExpectationFactory, std::forward<decltype(args)>(args)...); \
}                                                                                                                                                  \
(defaultExpectationFactory_, __VA_ARGS__)                                                                                                          \

#define DOTS_EXPECT_NAMED_CALL_SEQUENCE(defaultExpectationFactory_, sequence_, ...)                                                                    \
[&](auto defaultExpectationFactory, const ::testing::Sequence& sequence, auto&&... args) -> auto&                                                      \
{                                                                                                                                                      \
    return dots::testing::deprecated::details::expect_named_call_sequence(defaultExpectationFactory, sequence, std::forward<decltype(args)>(args)...); \
}                                                                                                                                                      \
(defaultExpectationFactory_, sequence_, __VA_ARGS__)

#endif
