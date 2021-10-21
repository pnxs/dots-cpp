#pragma once
#include <dots/testing/gtest/gtest.h>

namespace dots::testing::details
{
    template <typename... Expectations>
    struct TypedExpectationSet
    {
        static_assert(std::conjunction_v<std::is_constructible<::testing::Expectation, Expectations&>...>, "expectation bundle arguments must be Google Test expectations");

        TypedExpectationSet(Expectations&... expectations) : m_expectations{ std::tie(expectations...) }
        {
            /* do nothing */
        }

        TypedExpectationSet(const TypedExpectationSet& other) = default;
        TypedExpectationSet(TypedExpectationSet&& other) = default;
        ~TypedExpectationSet() = default;

        TypedExpectationSet& operator = (const TypedExpectationSet& rhs) = delete;
        TypedExpectationSet& operator = (TypedExpectationSet&& rhs) = delete;

    private:

        template <typename...>
        friend struct TypedExpectationSet;

        template <typename... PreviousExpectations, typename... NextExpectations, typename... ArgTail>
        friend auto expectation_sequence_recursive(const TypedExpectationSet<PreviousExpectations...>& previous, const TypedExpectationSet<NextExpectations...>& next, ArgTail&&... argTail);

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
        static constexpr bool is_compatible_action_v = is_compatible_action_t<T, Signature>::value;

        template <typename... OtherExpectations>
        void after(const TypedExpectationSet<OtherExpectations...>& other) const
        {
            std::apply([&other](auto&... expectations)
            {
                auto after = [&other](auto& expectation)
                {
                    std::apply([&expectation](auto&... previousExpectations)
                    {
                        (expectation.After(previousExpectations), ...);
                    }, other.m_expectations);
                };

                (after(expectations), ...);
            }, m_expectations);
        }

        template <typename Action>
        void willOnce(Action&& action) const
        {
            int cardinality = std::apply([this](auto&... expectations)
            {
                auto get_cardinality = [](auto& expectation)
                {
                    if (expectation.cardinality().IsSaturatedByCallCount(0))
                    {
                        return 0;
                    }
                    else if (expectation.cardinality().IsSaturatedByCallCount(1))
                    {
                        return 1;
                    }
                    else
                    {
                        throw std::logic_error{ "expectation sets only support actions on expectations with a cardinality of 0 or 1" };
                    }
                };

                return (get_cardinality(expectations) + ... + 0);
            }, m_expectations);

            if (cardinality == 0)
            {
                throw std::logic_error{ "expectation sets with a total cardinality of 0 cannot have a subsequent action" };
            }

            if constexpr (sizeof...(Expectations) == 1)
            {
                using expectation_t = std::tuple_element_t<0, std::tuple<Expectations...>>;
                constexpr bool IsCompatibleAction = is_compatible_action_v<Action, expectation_signature_t<expectation_t>>;
                static_assert(IsCompatibleAction, "action for set of a single expectation must be compatible with signature of expectation function or be trivially invocable");

                if constexpr (IsCompatibleAction)
                {
                    std::apply([action{ std::forward<Action>(action) }](auto& expectation)
                    {
                        expectation.WillOnce(::testing::DoAll(std::move(action)));
                    }, m_expectations);
                }
            }
            else if constexpr (sizeof...(Expectations) >= 2)
            {
                constexpr bool IsTrivialAction = std::is_invocable_v<Action>;
                static_assert(IsTrivialAction, "action for set of multiple expectations must be trivially invocable");

                if constexpr (IsTrivialAction)
                {
                    std::apply([cardinality, action{ std::forward<Action>(action) }](auto&... expectations)
                    {
                        auto deferred_action = [cardinality, actionData{ std::make_shared<std::pair<Action, int>>(std::move(action), 0) }]()
                        {
                            if (auto& [action, callCount] = *actionData; ++callCount == cardinality)
                            {
                                action();
                            }
                        };

                        auto add_deferred_action = [&deferred_action](auto& expectation)
                        {
                            if (!expectation.cardinality().IsSaturatedByCallCount(0))
                            {
                                expectation.WillOnce(::testing::DoAll(deferred_action));
                            }
                        };

                        (add_deferred_action(expectations), ...);
                    }, m_expectations);
                }
            }
        }

        std::tuple<Expectations&...> m_expectations;
    };

    template <typename T>
    struct is_typed_expectation_set : std::false_type {};

    template <typename... Expectations>
    struct is_typed_expectation_set<TypedExpectationSet<Expectations...>> : std::true_type {};

    template <typename T>
    using is_typed_expectation_set_t = typename is_typed_expectation_set<T>::type;

    template <typename T>
    constexpr bool is_typed_expectation_set_v = is_typed_expectation_set_t<T>::value;

    template <typename... PreviousExpectations, typename... NextExpectations, typename... ArgTail>
    auto expectation_sequence_recursive(const TypedExpectationSet<PreviousExpectations...>& previous, const TypedExpectationSet<NextExpectations...>& next, ArgTail&&... argTail)
    {
        next.after(previous);

        if constexpr (sizeof...(argTail) > 0)
        {
            auto argTailRefs = std::forward_as_tuple(std::forward<decltype(argTail)>(argTail)...);
            using arg_tail_head_t = std::tuple_element_t<0, decltype(argTailRefs)>;

            if constexpr (is_typed_expectation_set_v<arg_tail_head_t>)
            {
                return expectation_sequence_recursive(next, std::forward<decltype(argTail)>(argTail)...);
            }
            else if constexpr (std::is_constructible_v<::testing::Expectation, arg_tail_head_t>)
            {
                return std::apply([&next](auto& argTailHead, auto&&... argTailTail) -> auto
                {
                    return expectation_sequence_recursive(next, TypedExpectationSet{ argTailHead }, std::forward<decltype(argTailTail)>(argTailTail)...);
                }, argTailRefs);
            }
            else
            {
                next.willOnce(std::forward<arg_tail_head_t>(std::get<0>(argTailRefs)));

                if constexpr (sizeof...(argTail) > 1)
                {
                    return std::apply([&next](auto&/* argTailHead*/, auto&&... argTailTail) -> auto
                    {
                        return expectation_sequence_recursive(next, std::forward<decltype(argTailTail)>(argTailTail)...);
                    }, argTailRefs);
                }
                else
                {
                    return next;
                }
            }
        }
        else
        {
            return next;
        }
    }

    template <typename... PreviousExpectations, typename NextExpectation, typename... ArgTail, std::enable_if_t<!is_typed_expectation_set_v<NextExpectation>, int> = 0>
    auto expectation_sequence_recursive(const TypedExpectationSet<PreviousExpectations...>& previous, NextExpectation& next, ArgTail&&... argTail)
    {
        return expectation_sequence_recursive(previous, TypedExpectationSet{ next }, std::forward<decltype(argTail)>(argTail)...);
    }

    template <typename ArgHead, typename... ArgTail>
    auto expectation_sequence(ArgHead&& argHead, ArgTail&&... argTail)
    {
        if constexpr (std::is_invocable_v<decltype(argHead)>)
        {
            auto expectation = expectation_sequence_recursive(TypedExpectationSet{}, std::forward<decltype(argTail)>(argTail)...);
            std::invoke(std::forward<decltype(argHead)>(argHead));

            return expectation;
        }
        else
        {
            return expectation_sequence_recursive(TypedExpectationSet{}, std::forward<decltype(argHead)>(argHead), std::forward<decltype(argTail)>(argTail)...);
        }
    }
}

/*!
 * @brief Create a sequence of Google Test expectations.
 *
 * This function-like macro imposes ordering requirements to an
 * arbitrary number of expectation arguments as if the .After clause
 * was used. This means that all expectations will be required to occur
 * in the order they are given.
 *
 * Expectation arguments can be any expectations created by the Google
 * Test EXPECT_CALL() macro. This includes results that are produced by
 * macros which are based on that macro, such as DOTS_EXPECT_PUBLISH().
 *
 * To create a partial ordering, expectations can also be grouped
 * together into typed expectation sets (see DOTS_EXPECTATION_SET()).
 *
 * Every expectation argument can optionally be followed up by an
 * action (e.g. a lambda) that will be invoked when the expectation is
 * satisfied.
 *
 * The macro optionally also accepts a parameterless action as its
 * first argument, which will be invoked once the sequence was created.
 *
 * This macro is intended to be used in the top level of a Google Test
 * test case.
 *
 * @code{.cpp}
 * DOTS_EXPECTATION_SEQUENCE(
 *     [&]
 *     {
 *         // initial action invoked after sequence creation
 *     },
 *     EXPECT_CALL(...), // #1
 *     [&]
 *     {
 *         // action invoked after satisfaction of #1
 *     },
 *     EXPECT_CALL(...).Times(0), // #2
 *     EXPECT_CALL(...).Times(2), // #3
 *     EXPECT_DOTS_PUBLISH(...), // #4
 *     [&](const dots::Event<>& event)
 *     {
 *         // action invoked after satisfaction of #4
 *     }
 *     ...
 * );
 * @endcode
 *
 * @attention Expectation arguments with a cardinality other than 1
 * (e.g. by using the .Times(2) clause) cannot have a subsequent
 * action. If the argument is an expectation set, all members but one
 * may have a cardinality of 0.
 *
 * @remark Actions for satisfied expectations can be any objects that
 * are either trivially invocable (i.e. without arguments) or are
 * compatible with the signature of the mock function the expectation
 * was created for.
 *
 * @param args The list of ordered Google Test expectations and actions
 * to create a sequence from.
 */
#define DOTS_EXPECTATION_SEQUENCE                                                        \
[&](auto&&... args)                                                                      \
{                                                                                        \
    dots::testing::details::expectation_sequence(std::forward<decltype(args)>(args)...); \
}

/*!
 * @brief Create a typed set of Google Test expectations.
 *
 * This function-like macro groups an arbitrary number of Google Test
 * expectations into a typed expectation set, which can be used as an
 * argument for DOTS_EXPECTATION_SEQUENCE() to create partial
 * expectation orderings.
 *
 * This means that every expectation in the set may occur in any order
 * relative to one another, but will be required to occur after the
 * expectation preceding the set.
 *
 * Likewise, an expectation succeeding the set will be required to
 * occur after all its members.
 *
 * The set can optionally be followed by a parameterless action that
 * will be invoked once all expectations were satisfied.
 *
 * @code{.cpp}
 * DOTS_EXPECTATION_SEQUENCE(
 *     EXPECT_CALL(...), // #1
 *     EXPECT_CALL(...), // #2
 *     DOTS_EXPECTATION_SET(
 *         // members may occur in any order
 *         EXPECT_DOTS_PUBLISH(...), // #3a
 *         EXPECT_DOTS_PUBLISH(...).Times(0), // #3b
 *         EXPECT_CALL(...) // #3c
 *     ),
 *     [&]
 *     {
 *         // action invoked after satisfaction of #3a, #3b and #3c
 *     },
 *     EXPECT_CALL(...), // #4
 *     ...
 * );
 * @endcode
 *
 * @remark The exact return type of this macro is an implementation
 * detail and results should only be used as arguments to
 * DOTS_EXPECTATION_SEQUENCE(). However, the returned object can
 * assumed to be copyable and movable, but not assignable.
 *
 * @param args The list of Google Test expectations to create a set of.
 *
 * @return The typed expectation set of the expectations given in @p
 * args.
 */
#define DOTS_EXPECTATION_SET                                                                     \
[&](auto&&... args) -> auto                                                                      \
{                                                                                                \
    return dots::testing::details::TypedExpectationSet{ std::forward<decltype(args)>(args)... }; \
}
