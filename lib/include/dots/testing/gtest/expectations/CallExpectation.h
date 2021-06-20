#pragma once
#include <dots/testing/gtest/gtest.h>

namespace dots::testing::details
{
    template <typename T, typename = void>
    struct gtest_expectation_signature {};

    template <typename F>
    struct gtest_expectation_signature<::testing::internal::TypedExpectation<F>> { using type = F; };

    template <typename T>
    using gtest_expectation_signature_t = typename gtest_expectation_signature<T>::type;

    template <typename T, typename = void>
    struct is_compatible_gtest_action : std::false_type {};

    template <typename T, typename... Args>
    struct is_compatible_gtest_action<T, void(Args...)> : std::disjunction<std::is_invocable<T>, std::is_invocable<T, Args...>> {};

    template <typename T, typename Signature>
    using is_compatible_gtest_action_t = typename is_compatible_gtest_action<T, Signature>::type;

    template <typename T, typename Signature>
    constexpr bool is_compatible_gtest_action_v = is_compatible_gtest_action_t<T, Signature>::value;

    template <typename ExpectCall, typename ArgHead, typename... ArgTail>
    auto& expect_consecutive_call_sequence_recursive(ExpectCall expectCall, ArgHead&& argHead, ArgTail&&... argTail)
    {
        auto& callExpectation = expectCall(std::forward<decltype(argHead)>(argHead));
        using call_expectation_t = std::decay_t<decltype(callExpectation)>;

        if constexpr (sizeof...(argTail) > 0)
        {
            auto argTailTuple = std::make_tuple(std::forward<decltype(argTail)>(argTail)...);
            using arg_tail_head_t = std::tuple_element_t<0, decltype(argTailTuple)>;

            if constexpr (is_compatible_gtest_action_v<arg_tail_head_t, gtest_expectation_signature_t<call_expectation_t>>)
            {
                auto action = std::forward<arg_tail_head_t>(std::get<0>(argTailTuple));
                return callExpectation.WillOnce(::testing::DoAll(
                    [expectCall{ std::move(expectCall) }, argTailTuple{ std::move(argTailTuple) }](auto&&...)
                    {
                        if constexpr (sizeof...(argTail) > 1)
                        {
                            std::apply([expectCall{ std::move(expectCall) }](auto&&/* argTailHead*/, auto&&... argTailTail) -> auto&
                            {
                                return expect_consecutive_call_sequence_recursive(expectCall, std::forward<decltype(argTailTail)>(argTailTail)...);
                            }, argTailTuple);
                        }
                    },
                    std::move(action)
                ));
            }
            else
            {
                return callExpectation.WillOnce(::testing::DoAll([expectCall{ std::move(expectCall) }, argTailTuple{ std::move(argTailTuple) }](auto&&...)
                {
                    std::apply([expectCall{ std::move(expectCall) }](auto&&... argTailTail) -> auto&
                    {
                        return expect_consecutive_call_sequence_recursive(expectCall, std::forward<decltype(argTailTail)>(argTailTail)...);
                    }, argTailTuple);
                }));
            }
        }
        else
        {
            return callExpectation;
        }
    }

    template <typename ExpectCall, typename ArgHead, typename... ArgTail>
    auto& expect_named_call_sequence_recursive(ExpectCall& expectCall, ArgHead&& argHead, ArgTail&&... argTail)
    {
        auto& callExpectation = expectCall(std::forward<decltype(argHead)>(argHead));
        using call_expectation_t = std::decay_t<decltype(callExpectation)>;

        if constexpr (sizeof...(argTail) > 0)
        {
            auto argTailRefs = std::forward_as_tuple(std::forward<decltype(argTail)>(argTail)...);
            using arg_tail_head_t = std::tuple_element_t<0, decltype(argTailRefs)>;

            if constexpr (is_compatible_gtest_action_v<arg_tail_head_t, gtest_expectation_signature_t<call_expectation_t>>)
            {
                callExpectation.WillOnce(::testing::DoAll(std::forward<arg_tail_head_t>(std::get<0>(argTailRefs))));

                if constexpr (sizeof...(argTail) > 1)
                {
                    return std::apply([&expectCall](auto&&/* argTailHead*/, auto&&... argTailTail) -> auto&
                    {
                        return expect_named_call_sequence_recursive(expectCall, std::forward<decltype(argTailTail)>(argTailTail)...);
                    }, argTailRefs);
                }
                else
                {
                    return callExpectation;
                }
            }
            else
            {
                (void)callExpectation;
                return expect_named_call_sequence_recursive(expectCall, std::forward<decltype(argTail)>(argTail)...);
            }
        }
        else
        {
            return callExpectation;
        }
    }


    template <typename ExpectCall, typename ArgHead, typename... ArgTail>
    auto& expect_consecutive_call_sequence(ExpectCall expectCall, ArgHead&& argHead, ArgTail&&... argTail)
    {
        if constexpr (std::is_invocable_v<decltype(argHead)>)
        {
            auto& callExpectation = expect_consecutive_call_sequence_recursive(expectCall, std::forward<decltype(argTail)>(argTail)...);
            std::invoke(std::forward<decltype(argHead)>(argHead));

            return callExpectation;
        }
        else
        {
            return expect_consecutive_call_sequence_recursive(expectCall, std::forward<decltype(argHead)>(argHead), std::forward<decltype(argTail)>(argTail)...);
        }
    }

    template <typename ExpectCall, typename ArgHead, typename... ArgTail>
    auto& expect_named_call_sequence(ExpectCall expectCall, ArgHead&& argHead, ArgTail&&... argTail)
    {
        if constexpr (std::is_invocable_v<decltype(argHead)>)
        {
            auto& callExpectation = expect_named_call_sequence_recursive(expectCall, std::forward<decltype(argTail)>(argTail)...);
            std::invoke(std::forward<decltype(argHead)>(argHead));

            return callExpectation;
        }
        else
        {
            return expect_named_call_sequence_recursive(expectCall, std::forward<decltype(argHead)>(argHead), std::forward<decltype(argTail)>(argTail)...);
        }
    }
}

#define DOTS_EXPECT_CONSECUTIVE_CALL_SEQUENCE(expectCall_, ...)                                                                               \
[&](auto expectCall, auto&&... args) -> auto&                                                                                                 \
{                                                                                                                                             \
    return dots::testing::details::expect_consecutive_call_sequence(expectCall, std::forward<decltype(args)>(args)...);                       \
}                                                                                                                                             \
(expectCall_, __VA_ARGS__)

#define DOTS_EXPECT_NAMED_CALL_SEQUENCE(expectCall_, sequence_, ...)                                                                     \
[&](const ::testing::Sequence& sequence, auto&&... args) -> auto&                                                                        \
{                                                                                                                                        \
    auto expect_call = [&](auto&& arg) -> auto&                                                                                          \
    {                                                                                                                                    \
        return expectCall_(std::forward<decltype(arg)>(arg)).InSequence(sequence);                                                       \
    };                                                                                                                                   \
                                                                                                                                         \
    return dots::testing::details::expect_named_call_sequence(expect_call, std::forward<decltype(args)>(args)...); \
}                                                                                                                                        \
(sequence_, __VA_ARGS__)
