#pragma once
#include <map>
#include <vector>
#include <type_traits>
#include <dots/testing/gtest/gtest.h>
#include <dots/dots.h>
#include <dots/HostTransceiver.h>
#include <dots/io/channels/LocalListener.h>
#include <dots/testing/gtest/expectations/CallExpectation.h>
#include <dots/testing/gtest/expectations/PublishExpectation.h>
#include <dots/tools/logging.h>

#if defined(DOTS_ENABLE_DEPRECATED_TESTING_SUPPORT)

namespace dots::testing
{
    struct PublishTestBase : ::testing::Test
    {
        using mock_subscription_handler_t = ::testing::MockFunction<void(const io::Transmission&)>;

        PublishTestBase(asio::io_context& ioContext = io::global_io_context(), std::string hostName = "dots-test-host") :
            m_host{ std::move(hostName), ioContext },
            m_globalGuest(nullptr),
            m_spoofGuest(std::nullopt),
            m_localListener{ m_host.listen<dots::io::LocalListener>() }
        {
            PublishTestBase::ioContext().restart();

            // disable verbose logging unless overriden by the user
            if (::getenv("LOGGING_LEVEL") == nullptr)
            {
                tools::loggingFrontend().setLogLevel(tools::Level::warn);
            }

            // note: global guest currently has to be always connected
            globalGuest();
        }

        PublishTestBase(const PublishTestBase& other) = delete;
        PublishTestBase(PublishTestBase&& other) = delete;

        ~PublishTestBase()
        {
            if (m_globalGuest != nullptr)
            {
                transceiver("dots-test-guest", true);
                m_globalGuest = nullptr;
            }
        }

        PublishTestBase& operator = (const PublishTestBase& rhs) = delete;
        PublishTestBase& operator = (PublishTestBase&& rhs) = delete;

    protected:

        using mock_subscription_handlers_t = std::map<const type::Descriptor<>*, mock_subscription_handler_t>;

        const asio::io_context& ioContext() const
        {
            return m_host.ioContext();
        }

        asio::io_context& ioContext()
        {
            return m_host.ioContext();
        }

        void processEvents()
        {
            m_host.ioContext().poll();
            m_host.ioContext().restart();
        }

        const HostTransceiver& host() const
        {
            return m_host;
        }

        HostTransceiver& host()
        {
            return m_host;
        }

        GuestTransceiver& globalGuest()
        {
            if (m_globalGuest == nullptr)
            {
                m_globalGuest = &transceiver("dots-global-guest", true);
                m_globalGuest->open<io::LocalChannel>(io::global_publish_types(), io::global_subscribe_types(), std::optional<std::string>{ std::nullopt }, m_localListener);
                processEvents();
            }

            return *m_globalGuest;
        }

        GuestTransceiver& spoofGuest()
        {
            if (m_spoofGuest == std::nullopt)
            {
                m_spoofGuest.emplace("dots-spoof-guest", m_host.ioContext());
                m_spoofGuest->open<io::LocalChannel>(m_localListener);
                processEvents();
            }

            return *m_spoofGuest;
        }

        auto mockSubscriptionHandlers() const -> const std::map<GuestTransceiver*, mock_subscription_handlers_t>&
        {
            return m_mockSubscriptionHandlers;
        }

        auto mockSubscriptionHandlers() -> std::map<GuestTransceiver*, mock_subscription_handlers_t>&
        {
            return m_mockSubscriptionHandlers;
        }

        mock_subscription_handler_t& getMockSubscriptionHandler(GuestTransceiver& guest, const type::StructDescriptor<>& descriptor)
        {
            auto [itGuest, emplacedGuest] = m_mockSubscriptionHandlers.try_emplace(&guest);

            if (emplacedGuest)
            {
                GuestTransceiver* guestPtr = itGuest->first;

                if (guestPtr != m_globalGuest && (m_spoofGuest == std::nullopt || &*m_spoofGuest != guestPtr))
                {
                    guestPtr->open<io::LocalChannel>(m_localListener);
                    processEvents();
                }
            }

            auto [itSubscriptionHandler, emplacedSubscriptionHandler] = itGuest->second.try_emplace(&descriptor);

            if (emplacedSubscriptionHandler)
            {
                m_subscriptions.emplace_back(m_host.subscribe(descriptor, [this, mockHandler = &itSubscriptionHandler->second](const io::Transmission& transmission)
                {
                    // delay invocation of the mock handler, so that a potential self
                    // update for the transmission is already queued for execution. this
                    // ensures that if a user has specified a reactive action, they are
                    // able to post the creation of a depending stimulus or expectation
                    // "behind" the self update.
                    asio::post(ioContext(), [mockHandler, transmission = io::Transmission{ transmission.header(), transmission.instance() }]
                    {
                        (*mockHandler).AsStdFunction()(transmission);
                    });
                }));
            }

            return itSubscriptionHandler->second;
        }

    private:

        HostTransceiver m_host;
        GuestTransceiver* m_globalGuest;
        std::optional<GuestTransceiver> m_spoofGuest;
        io::LocalListener& m_localListener;
        std::map<GuestTransceiver*, mock_subscription_handlers_t> m_mockSubscriptionHandlers;
        std::vector<dots::Subscription> m_subscriptions;
    };

    struct publish_spoof_tag {};

    template <typename T>
    struct PublishSpoof : publish_spoof_tag
    {
        PublishSpoof(T instance, std::optional<dots::property_set_t> includedProperties = std::nullopt, bool remove = false) :
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

#define DOTS_MAKE_EXPECT_PUBLISH_MOCK_SUBSCRIPTION_HANDLER_RECEIVER(guestTransceiver_) [&](const auto& instance) -> auto& { return PublishTestBase::getMockSubscriptionHandler(guestTransceiver_, instance._descriptor()); }

#define DOTS_EXPECT_PUBLISH_FROM_GUEST(guestTransceiver_, ...) DOTS_MAKE_EXPECT_PUBLISH_AT_SUBSCRIBER(DOTS_MAKE_EXPECT_PUBLISH_MOCK_SUBSCRIPTION_HANDLER_RECEIVER(guestTransceiver_))(__VA_ARGS__)
#define DOTS_EXPECT_PUBLISH_SEQUENCE_FROM_GUEST(guestTransceiver_, ...) DOTS_EXPECT_PUBLISH_SEQUENCE_AT_SUBSCRIBER(DOTS_MAKE_EXPECT_PUBLISH_MOCK_SUBSCRIPTION_HANDLER_RECEIVER(guestTransceiver_), __VA_ARGS__)
#define DOTS_EXPECT_NAMED_PUBLISH_SEQUENCE_FROM_GUEST(guestTransceiver_, sequence_, ...) DOTS_EXPECT_NAMED_PUBLISH_SEQUENCE_AT_SUBSCRIBER(DOTS_MAKE_EXPECT_PUBLISH_MOCK_SUBSCRIPTION_HANDLER_RECEIVER(guestTransceiver_), sequence_, __VA_ARGS__)

#define DOTS_EXPECT_PUBLISH(...) DOTS_EXPECT_PUBLISH_FROM_GUEST(PublishTestBase::globalGuest(), __VA_ARGS__)
#define DOTS_EXPECT_PUBLISH_SEQUENCE(...) DOTS_EXPECT_PUBLISH_SEQUENCE_FROM_GUEST(PublishTestBase::globalGuest(), __VA_ARGS__)
#define DOTS_EXPECT_NAMED_PUBLISH_SEQUENCE(sequence_, ...) DOTS_EXPECT_NAMED_PUBLISH_SEQUENCE_FROM_GUEST(PublishTestBase::globalGuest(), sequence_, __VA_ARGS__)

#define DOTS_SPOOF_SINGLE_PUBLISH(publishSpoof_)                                                                                                 \
[this](auto&& publishSpoof)                                                                                                                      \
{                                                                                                                                                \
    constexpr bool IsStruct = std::is_base_of_v<dots::type::Struct, std::decay_t<decltype(publishSpoof)>>;                                       \
    constexpr bool IsPublishSpoof = std::is_base_of_v<dots::testing::publish_spoof_tag, std::decay_t<decltype(publishSpoof)>>;                   \
    static_assert(IsStruct || IsPublishSpoof, "publish spoof has to be either an instance of a DOTS struct type or of type PublishSpoof");       \
                                                                                                                                                 \
    if constexpr (IsStruct)                                                                                                                      \
    {                                                                                                                                            \
        for (auto& [guest, mockSubscriptionHandlers] : PublishTestBase::mockSubscriptionHandlers())                                              \
        {                                                                                                                                        \
            (void)mockSubscriptionHandlers;                                                                                                      \
            DOTS_EXPECT_PUBLISH_FROM_GUEST(*guest, publishSpoof).RetiresOnSaturation();                                                          \
        }                                                                                                                                        \
        PublishTestBase::spoofGuest().publish(publishSpoof, std::nullopt, false);                                                                \
    }                                                                                                                                            \
    else if constexpr (IsPublishSpoof)                                                                                                           \
    {                                                                                                                                            \
        for (auto& [guest, mockSubscriptionHandlers] : PublishTestBase::mockSubscriptionHandlers())                                              \
        {                                                                                                                                        \
            (void)mockSubscriptionHandlers;                                                                                                      \
            DOTS_EXPECT_PUBLISH_FROM_GUEST(                                                                                                      \
                *guest,                                                                                                                          \
                dots::testing::PublishExpectation{                                                                                               \
                    publishSpoof.instance,                                                                                                       \
                    publishSpoof.includedProperties,                                                                                             \
                    publishSpoof.remove                                                                                                          \
                }                                                                                                                                \
            ).RetiresOnSaturation();                                                                                                             \
        }                                                                                                                                        \
        PublishTestBase::spoofGuest().publish(publishSpoof.instance, publishSpoof.includedProperties, publishSpoof.remove);                      \
    }                                                                                                                                            \
}(publishSpoof_)

#define DOTS_SPOOF_PUBLISH(...)                                                             \
[this](auto&&... publishSpoofs)                                                             \
{                                                                                           \
    (DOTS_SPOOF_SINGLE_PUBLISH(std::forward<decltype(publishSpoofs)>(publishSpoofs)), ...); \
    PublishTestBase::processEvents();                                                       \
}(__VA_ARGS__)

#define DOTS_SPOOF_REMOVE(...)                                                                                                  \
[this](auto&&... instances)                                                                                                     \
{                                                                                                                               \
    constexpr bool AreStructs = std::conjunction_v<std::is_base_of<dots::type::Struct, std::decay_t<decltype(instances)>>...>;  \
    static_assert(AreStructs, "arguments to spoof remove have to be instances of DOTS struct types");                           \
                                                                                                                                \
    if constexpr (AreStructs)                                                                                                   \
    {                                                                                                                           \
        DOTS_SPOOF_PUBLISH(dots::testing::PublishSpoof{ std::forward<decltype(instances)>(instances), std::nullopt, true }...); \
    }                                                                                                                           \
}(__VA_ARGS__)

#endif
