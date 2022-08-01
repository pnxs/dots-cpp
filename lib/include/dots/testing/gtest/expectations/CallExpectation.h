// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <dots/testing/gtest/gtest.h>

#if defined(DOTS_ENABLE_DEPRECATED_TESTING_SUPPORT)

namespace dots::testing::details
{
    template <typename T, typename = void>
    struct is_compatible_gtest_action : std::false_type {};

    template <typename T, typename R, typename... Args>
    struct is_compatible_gtest_action<T, R(Args...)> : std::disjunction<std::is_invocable<T>, std::is_invocable<T, Args...>> {};

    template <typename T, typename Signature>
    using is_compatible_gtest_action_t = typename is_compatible_gtest_action<T, Signature>::type;

    template <typename T, typename Signature>
    constexpr bool is_compatible_gtest_action_v = is_compatible_gtest_action_t<T, Signature>::value;

    template <typename ExpectCallSignature, typename ExpectCall, typename ArgHead, typename... ArgTail>
    auto& expect_consecutive_call_sequence_recursive(ExpectCall expectCall, ArgHead&& argHead, ArgTail&&... argTail)
    {
        auto& callExpectation = expectCall(std::forward<decltype(argHead)>(argHead));

        if constexpr (sizeof...(argTail) > 0)
        {
            auto argTailTuple = std::make_tuple(std::forward<decltype(argTail)>(argTail)...);
            using arg_tail_head_t = std::tuple_element_t<0, decltype(argTailTuple)>;

            if constexpr (is_compatible_gtest_action_v<arg_tail_head_t, ExpectCallSignature>)
            {
                auto action = std::forward<arg_tail_head_t>(std::get<0>(argTailTuple));
                return callExpectation.WillOnce(::testing::DoAll(
                    [expectCall{ std::move(expectCall) }, argTailTuple{ std::move(argTailTuple) }](auto&&...)
                    {
                        if constexpr (sizeof...(argTail) > 1)
                        {
                            std::apply([expectCall{ std::move(expectCall) }](auto&&/* argTailHead*/, auto&&... argTailTail) -> auto&
                            {
                                return expect_consecutive_call_sequence_recursive<ExpectCallSignature>(expectCall, std::forward<decltype(argTailTail)>(argTailTail)...);
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
                        return expect_consecutive_call_sequence_recursive<ExpectCallSignature>(expectCall, std::forward<decltype(argTailTail)>(argTailTail)...);
                    }, argTailTuple);
                }));
            }
        }
        else
        {
            return callExpectation;
        }
    }

    template <typename ExpectCallSignature, typename ExpectCall, typename ArgHead, typename... ArgTail>
    auto& expect_named_call_sequence_recursive(ExpectCall& expectCall, ArgHead&& argHead, ArgTail&&... argTail)
    {
        auto& callExpectation = expectCall(std::forward<decltype(argHead)>(argHead));

        if constexpr (sizeof...(argTail) > 0)
        {
            auto argTailRefs = std::forward_as_tuple(std::forward<decltype(argTail)>(argTail)...);
            using arg_tail_head_t = std::tuple_element_t<0, decltype(argTailRefs)>;

            if constexpr (is_compatible_gtest_action_v<arg_tail_head_t, ExpectCallSignature>)
            {
                callExpectation.WillOnce(::testing::DoAll(std::forward<arg_tail_head_t>(std::get<0>(argTailRefs))));

                if constexpr (sizeof...(argTail) > 1)
                {
                    return std::apply([&expectCall](auto&&/* argTailHead*/, auto&&... argTailTail) -> auto&
                    {
                        return expect_named_call_sequence_recursive<ExpectCallSignature>(expectCall, std::forward<decltype(argTailTail)>(argTailTail)...);
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
                return expect_named_call_sequence_recursive<ExpectCallSignature>(expectCall, std::forward<decltype(argTail)>(argTail)...);
            }
        }
        else
        {
            return callExpectation;
        }
    }


    template <typename ExpectCallSignature, typename ExpectCall, typename ArgHead, typename... ArgTail>
    auto& expect_consecutive_call_sequence(ExpectCall expectCall, ArgHead&& argHead, ArgTail&&... argTail)
    {
        if constexpr (std::is_invocable_v<decltype(argHead)>)
        {
            auto& callExpectation = expect_consecutive_call_sequence_recursive<ExpectCallSignature>(expectCall, std::forward<decltype(argTail)>(argTail)...);
            std::invoke(std::forward<decltype(argHead)>(argHead));

            return callExpectation;
        }
        else
        {
            return expect_consecutive_call_sequence_recursive<ExpectCallSignature>(expectCall, std::forward<decltype(argHead)>(argHead), std::forward<decltype(argTail)>(argTail)...);
        }
    }

    template <typename ExpectCallSignature, typename ExpectCall, typename ArgHead, typename... ArgTail>
    auto& expect_named_call_sequence(ExpectCall expectCall, ArgHead&& argHead, ArgTail&&... argTail)
    {
        if constexpr (std::is_invocable_v<decltype(argHead)>)
        {
            auto& callExpectation = expect_named_call_sequence_recursive<ExpectCallSignature>(expectCall, std::forward<decltype(argTail)>(argTail)...);
            std::invoke(std::forward<decltype(argHead)>(argHead));

            return callExpectation;
        }
        else
        {
            return expect_named_call_sequence_recursive<ExpectCallSignature>(expectCall, std::forward<decltype(argHead)>(argHead), std::forward<decltype(argTail)>(argTail)...);
        }
    }
}

#define DOTS_EXPECT_CONSECUTIVE_CALL_SEQUENCE(expectCall_, expectCallSignature_, ...)                                                          \
[&](auto&&... args) -> auto&                                                                                                                   \
{                                                                                                                                              \
    auto expect_call = [&](auto&& arg) -> auto&                                                                                                \
    {                                                                                                                                          \
        return expectCall_(std::forward<decltype(arg)>(arg));                                                                                  \
    };                                                                                                                                         \
                                                                                                                                               \
    return dots::testing::details::expect_consecutive_call_sequence<expectCallSignature_>(expect_call, std::forward<decltype(args)>(args)...); \
}                                                                                                                                              \
(__VA_ARGS__)

#define DOTS_EXPECT_NAMED_CALL_SEQUENCE(expectCall_, expectCallSignature_, sequence_, ...)                                               \
[&](const ::testing::Sequence& sequence, auto&&... args) -> auto&                                                                        \
{                                                                                                                                        \
    auto expect_call = [&](auto&& arg) -> auto&                                                                                          \
    {                                                                                                                                    \
        return expectCall_(std::forward<decltype(arg)>(arg)).InSequence(sequence);                                                       \
    };                                                                                                                                   \
                                                                                                                                         \
    return dots::testing::details::expect_named_call_sequence<expectCallSignature_>(expect_call, std::forward<decltype(args)>(args)...); \
}                                                                                                                                        \
(sequence_, __VA_ARGS__)

#endif
