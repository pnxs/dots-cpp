#pragma once
#include <dots/testing/gtest/expectations/CallExpectation.h>
#include <dots/testing/gtest/matchers/TransmissionMatcher.h>

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