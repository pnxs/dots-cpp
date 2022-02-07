#pragma once
#include <dots/testing/gtest/matchers/EventMatcher.h>

#if defined(DOTS_ENABLE_DEPRECATED_TESTING_SUPPORT)

#include <dots/testing/gtest/matchers/TransmissionMatcher.h>
#include <dots/testing/gtest/expectations/CallExpectation.h>

namespace dots::testing
{
    struct publish_expectation_tag {};

    template <typename T>
    struct PublishExpectation : publish_expectation_tag
    {
        PublishExpectation(T instance, std::optional<dots::property_set_t> includedProperties = std::nullopt, bool remove = false) :
            instance{ std::move(instance) },
            includedProperties{ includedProperties },
            remove(remove)
        {
            /* do nothing */
        }

        T instance;
        std::optional<dots::property_set_t> includedProperties = std::nullopt;
        bool remove = false;
    };
}

#define DOTS_MAKE_EXPECT_PUBLISH_AT_SUBSCRIBER(mockSubscriptionHandlerRetriever_)                                                                                            \
[&](auto&&... publishExpectations) -> decltype(auto)                                                                                                                         \
{                                                                                                                                                                            \
    auto expect_publish_at_subscriber = [&](auto&& publishExpectation) -> decltype(auto)                                                                                     \
    {                                                                                                                                                                        \
        constexpr bool IsStruct = std::is_base_of_v<dots::type::Struct, std::decay_t<decltype(publishExpectation)>>;                                                         \
        constexpr bool IsPublishExpectation = std::is_base_of_v<dots::testing::publish_expectation_tag, std::decay_t<decltype(publishExpectation)>>;                         \
        static_assert(IsStruct || IsPublishExpectation, "publish expectation has to be either an instance of a DOTS struct type or of type PublishExpectation");             \
                                                                                                                                                                             \
        if constexpr (IsStruct)                                                                                                                                              \
        {                                                                                                                                                                    \
            const auto& instance = publishExpectation;                                                                                                                       \
            auto& mockSubscriptionHandler = mockSubscriptionHandlerRetriever_(instance);                                                                                     \
                                                                                                                                                                             \
            return EXPECT_CALL(mockSubscriptionHandler, Call(dots::testing::TransmissionEqual(instance, std::nullopt, false)));                                              \
        }                                                                                                                                                                    \
        else if constexpr (IsPublishExpectation)                                                                                                                             \
        {                                                                                                                                                                    \
            const auto& instance = publishExpectation.instance;                                                                                                              \
            auto& mockSubscriptionHandler = mockSubscriptionHandlerRetriever_(instance);                                                                                     \
                                                                                                                                                                             \
            return EXPECT_CALL(mockSubscriptionHandler, Call(dots::testing::TransmissionEqual(instance, publishExpectation.includedProperties, publishExpectation.remove))); \
        }                                                                                                                                                                    \
        else                                                                                                                                                                 \
        {                                                                                                                                                                    \
            auto&& instance = std::declval<dots::type::Struct>();                                                                                                            \
            auto&& mockSubscriptionHandler = std::declval<::testing::MockFunction<void(const dots::io::Transmission&)>>();                                                   \
                                                                                                                                                                             \
            return EXPECT_CALL(mockSubscriptionHandler, Call(dots::testing::TransmissionEqual(instance, std::nullopt, false)));                                              \
        }                                                                                                                                                                    \
   };                                                                                                                                                                        \
                                                                                                                                                                             \
    if constexpr (sizeof...(publishExpectations) == 1)                                                                                                                       \
    {                                                                                                                                                                        \
        return (expect_publish_at_subscriber(std::forward<decltype(publishExpectations)>(publishExpectations)), ...);                                                        \
    }                                                                                                                                                                        \
    else                                                                                                                                                                     \
    {                                                                                                                                                                        \
        return std::tie(expect_publish_at_subscriber(std::forward<decltype(publishExpectations)>(publishExpectations))...);                                                  \
    }                                                                                                                                                                        \
}                                                                                                                                                                            \

#define DOTS_EXPECT_PUBLISH_AT_SUBSCRIBER(mockSubscriptionHandlerRetriever_, publishExpectation_) DOTS_MAKE_EXPECT_PUBLISH_AT_SUBSCRIBER(mockSubscriptionHandlerRetriever_)(publishExpectation_)
#define DOTS_EXPECT_PUBLISH_SEQUENCE_AT_SUBSCRIBER(mockSubscriptionHandlerRetriever_, ...) DOTS_EXPECT_CONSECUTIVE_CALL_SEQUENCE(DOTS_MAKE_EXPECT_PUBLISH_AT_SUBSCRIBER(mockSubscriptionHandlerRetriever_), void(const dots::io::Transmission&), __VA_ARGS__)
#define DOTS_EXPECT_NAMED_PUBLISH_SEQUENCE_AT_SUBSCRIBER(mockSubscriptionHandlerRetriever_, sequence_, ...) DOTS_EXPECT_NAMED_CALL_SEQUENCE(DOTS_MAKE_EXPECT_PUBLISH_AT_SUBSCRIBER(mockSubscriptionHandlerRetriever_), void(const dots::io::Transmission&), sequence_, __VA_ARGS__)

#endif

namespace dots::testing
{
    using mock_subscription_handler_t = ::testing::MockFunction<void(const Event<>&)>;
}

#define IMPL_EXPECT_DOTS_PUBLISH_AT_SUBSCRIBER                                                                                                                                            \
[](dots::testing::mock_subscription_handler_t& mockSubscriptionHandler, auto&& instance, std::optional<dots::property_set_t> includedProperties, bool remove, bool isFromMyself) -> auto& \
{                                                                                                                                                                                         \
    return EXPECT_CALL(mockSubscriptionHandler, Call(dots::testing::EventEqual(std::forward<decltype(instance)>(instance), includedProperties, remove, isFromMyself)));                   \
}

/*!
 * @brief Create a Google Test expectation for a DOTS publish event at
 * a specific subscription handler.
 *
 * This function-like macro creates an expectation that a given
 * instance will be published and result in a corresponding DOTS event
 * at a given subscriber.
 *
 * The expectation will be satisfied if the published (i.e.
 * transmitted) instance of the event compares equal for the given
 * property set (see also dots::testing::EventEqual()).
 *
 * @remark This macro is usually used indirectly by higher-level macros
 * such as EXPECT_DOTS_PUBLISH_AT_TRANSCEIVER() or
 * EXPECT_DOTS_PUBLISH().
 *
 * @remark The resulting expectation can either be used individually or
 * as part of a DOTS_EXPECTATION_SEQUENCE().
 *
 * @remark This macro requires events to be create or update events.
 * Expectations for remove events can be created by the
 * EXPECT_DOTS_REMOVE_AT_SUBSCRIBER() macro.
 *
 * @param mockSubscriptionHandler The mock subscription handler where
 * the event is expected to occur.
 *
 * @param instance The instance to compare the transmitted instance in
 * the event to.
 *
 * @param includedProperties The property set to include in the
 * equality comparison. If no set is given, the valid property set of
 * @p instance will be used.
 *
 * @return auto& A reference to the created Google Test expectation.
 */
#define EXPECT_DOTS_PUBLISH_AT_SUBSCRIBER                                                                                                                                \
[](dots::testing::mock_subscription_handler_t& mockSubscriptionHandler, auto&& instance, std::optional<dots::property_set_t> includedProperties = std::nullopt) -> auto& \
{                                                                                                                                                                        \
    return IMPL_EXPECT_DOTS_PUBLISH_AT_SUBSCRIBER(mockSubscriptionHandler, std::forward<decltype(instance)>(instance), includedProperties, false, false);                \
}

/*!
 * @brief Create a Google Test expectation for a DOTS remove event at a
 * specific subscription handler.
 *
 * This function-like macro creates an expectation that a given
 * instance will be removed and result in a corresponding DOTS event at
 * a given subscriber.
 *
 * The expectation will be satisfied if the published (i.e.
 * transmitted) instance of the event compares equal for the given
 * property set (see also dots::testing::EventEqual()).
 *
 * @remark This macro is usually used indirectly by higher-level macros
 * such as EXPECT_DOTS_REMOVE_AT_TRANSCEIVER() or EXPECT_DOTS_REMOVE().
 *
 * @remark The resulting expectation can either be used individually or
 * as part of a DOTS_EXPECTATION_SEQUENCE().
 *
 * @remark This macro requires events to be remove events. Expectations
 * for create or update events can be created by the
 * EXPECT_DOTS_PUBLISH_AT_SUBSCRIBER() macro.
 *
 * @param mockSubscriptionHandler The mock subscription handler where
 * the event is expected to occur.
 *
 * @param instance The instance to compare the transmitted instance in
 * the event to.
 *
 * @param includedProperties The property set to include in the
 * equality comparison. If no set is given, the valid property set of
 * @p instance will be used.
 *
 * @return auto& A reference to the created Google Test expectation.
 */
#define EXPECT_DOTS_REMOVE_AT_SUBSCRIBER                                                                                                                                 \
[](dots::testing::mock_subscription_handler_t& mockSubscriptionHandler, auto&& instance, std::optional<dots::property_set_t> includedProperties = std::nullopt) -> auto& \
{                                                                                                                                                                        \
    return IMPL_EXPECT_DOTS_PUBLISH_AT_SUBSCRIBER(mockSubscriptionHandler, std::forward<decltype(instance)>(instance), includedProperties, true, false);                 \
}
