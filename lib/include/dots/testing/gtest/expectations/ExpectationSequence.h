#pragma once
#include <dots/testing/gtest/gtest.h>
#include <dots/testing/gtest/expectations/ExpectationTraits.h>

namespace dots::testing::details
{
    template <typename Expectation, typename... ArgTail>
    auto& expectation_sequence_recursive(const ::testing::Sequence& sequence, Expectation&& expectation, ArgTail&&... argTail)
    {
        static_assert(is_expectation_v<Expectation>, "expectation sequence argument must be a Google Test expectation");
        expectation.InSequence(sequence);

        if constexpr (sizeof...(argTail) > 0)
        {
            auto argTailRefs = std::forward_as_tuple(std::forward<decltype(argTail)>(argTail)...);
            using arg_tail_head_t = std::tuple_element_t<0, decltype(argTailRefs)>;

            if constexpr (is_compatible_action_v<arg_tail_head_t, expectation_signature_t<std::decay_t<Expectation>>>)
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
