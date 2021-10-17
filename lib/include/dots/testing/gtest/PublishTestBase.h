#pragma once
#include <map>
#include <vector>
#include <type_traits>
#include <dots/testing/gtest/gtest.h>
#include <dots/dots.h>
#include <dots/HostTransceiver.h>
#include <dots/io/channels/LocalListener.h>
#include <dots/testing/gtest/expectations/ExpectationSequence.h>
#include <dots/testing/gtest/expectations/PublishExpectation.h>
#include <dots/tools/logging.h>

#if defined(DOTS_ENABLE_DEPRECATED_SEQUENCE_SUPPORT)

namespace dots::testing
{
    struct PublishTestBase : ::testing::Test
    {
        using mock_subscription_handler_t = ::testing::MockFunction<void(const io::Transmission&)>;

        PublishTestBase(boost::asio::io_context& ioContext = io::global_io_context(), std::string hostName = "dots-test-host") :
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

        const boost::asio::io_context& ioContext() const
        {
            return m_host.ioContext();
        }

        boost::asio::io_context& ioContext()
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
                    boost::asio::post(ioContext(), [mockHandler, transmission = io::Transmission{ transmission.header(), transmission.instance() }]
                    {
                        (*mockHandler).AsStdFunction()(transmission);
                    });
                }));
            }

            return itSubscriptionHandler->second;
        }

        template <typename T>
        mock_subscription_handler_t& getMockSubscriptionHandler(GuestTransceiver& guest, T&& instance)
        {
            constexpr bool IsStruct = std::is_base_of_v<type::Struct, std::decay_t<T>>;
            static_assert(IsStruct, "instance type T has to be a DOTS struct type");

            if constexpr (IsStruct)
            {
                return getMockSubscriptionHandler(guest, std::forward<T>(instance)._descriptor());
            }
            else
            {
                auto&& mockSubscriptionHandler = std::declval<::testing::MockFunction<void(const io::Transmission&)>>();
                return mockSubscriptionHandler;
            }
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

#else

namespace dots::testing
{
    /*!
     * @class PublishTestBase PublishTestBase.h
     * <dots/testing/gtest/PublishTestBase.h>
     *
     * @brief Base class for Google Test suites that test components based
     * on DOTS events.
     *
     * Deriving from this class provides Google Test support for components
     * that rely on DOTS events (see dots::Event) via subscriptions or are
     * expected to produce them by publishing instances.
     *
     * Upon construction, the class creates a local DOTS environment,
     * including a DOTS host. Expectations and spoofs of DOTS events can
     * then be created via corresponding test macros (e.g.
     * EXPECT_DOTS_PUBLISH() and SPOOF_DOTS_PUBLISH() respectively).
     *
     * @attention Unless otherwise required, it is recommended to write
     * test cases in the "sequence style" by utilizing the
     * DOTS_EXPECTATION_SEQUENCE() macro.
     *
     * @attention If possible, this class should always be used with the
     * global guest transceiver. This should be sufficient for the majority
     * of use cases, as well as usually result in the most clear and least
     * verbose test cases.
     *
     * @warning This class is intended to be used as a base class only.
     */
    struct PublishTestBase : ::testing::Test
    {
        /*!
         * @brief Set up the DOTS test environment.
         *
         * Calling this constructor from a derived class initializes the DOTS
         * test environment.
         *
         * Similar to an "actual" DOTS application, the test environment is
         * running asynchronously in an event loop. This is because components
         * based on DOTS are inherently asynchronous and also often rely on
         * other asynchronous mechanisms, such as posts or timer events.
         *
         * The event loop defaults to the global event loop, but can be
         * provided by the user if necessary.
         *
         * After the constructor has returned, the test environment will have
         * been initialized and be ready for use.
         *
         * @warning The given IO context must match the IO context used by the
         * components that are part of the test.
         *
         * @remark To reduce clutter in the test output, the constructor sets
         * the logging level to tools::Level::warn. This can be overridden by
         * setting the LOGGING_LEVEL environment variable.
         *
         * @param ioContext The ASIO IO context (i.e. the "event loop") to use.
         *
         * @param hostName The name the host will use to identify itself.
         */
        PublishTestBase(boost::asio::io_context& ioContext = io::global_io_context(), std::string hostName = "dots-test-host") :
            m_host{ std::move(hostName), ioContext },
            m_globalGuest(nullptr),
            m_spoofGuest(std::nullopt),
            m_localListener{ m_host.listen<io::LocalListener>() }
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

        /*!
         * @brief Tear down the DOTS test environment.
         *
         * Calling the destructor from a derived class cleans up the test
         * environment. This includes the teardown of all mock subscription
         * handlers, as well as the destruction of the host and all guest
         * transceivers.
         *
         * In particular, the global guest transceiver will be reset if it has
         * been used.
         */
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

        /*!
         * @brief Get the currently used IO context.
         *
         * Note that this is the same IO context that was given in
         * PublishTestBase().
         *
         * @return const boost::asio::io_context& A reference to the currently
         * used IO context.
         */
        const boost::asio::io_context& ioContext() const
        {
            return m_host.ioContext();
        }

        /*!
         * @brief Get the currently used IO context.
         *
         * Note that this is the same IO context that was given in
         * PublishTestBase().
         *
         * @return boost::asio::io_context& A reference to the currently
         * used IO context.
         */
        boost::asio::io_context& ioContext()
        {
            return m_host.ioContext();
        }

        /*!
         * @brief Process all outstanding events.
         *
         * Calling this function will process all outstanding events (i.e.
         * ready handlers) in the event loop (i.e. the IO context), until it
         * has been stopped or there are no more ready handlers.
         *
         * This function will never block and in particular not wait for any
         * active timers to end.
         *
         * Technically, this effectively calls boost::asio::io_context::poll()
         * followed by a boost::asio::io_context::restart().
         *
         */
        void processEvents()
        {
            m_host.ioContext().poll();
            m_host.ioContext().restart();
        }

        /*!
         * @brief Process events for a specific duration.
         *
         * Calling this function will process the event loop (i.e. the IO
         * context) until all work has finished and there are no more handlers
         * to be dispatched, until the context has been stopped, or until the
         * given duration has elapsed.
         *
         * In particular, if there are any active timers, this function will
         * block up to the given duration and wait for the timers to end.
         *
         * Technically, this effectively calls
         * boost::asio::io_context::run_for() followed by a
         * boost::asio::io_context::restart().
         *
         * @tparam Rep An arithmetic type representing the number of ticks.
         *
         * @tparam Period A std::ratio representing the tick period (i.e. the
         * number of second's fractions per tick).
         *
         * @param duration The duration for which the call may block.
         */
        template <typename Rep, typename Period>
        void processEvents(std::chrono::duration<Rep, Period> duration)
        {
            m_host.ioContext().run_for(std::chrono::duration_cast<std::chrono::microseconds>(duration));
            m_host.ioContext().restart();
        }

        /*!
         * @brief Get the DOTS test host transceiver.
         *
         * @attention Note that it should rarely be required to access the host
         * manually. In particular, publishing instances should always be done
         * via the spoof macros, such as SPOOF_DOTS_PUBLISH().
         *
         * @return const HostTransceiver& A reference to the DOTS test host.
         */
        const HostTransceiver& host() const
        {
            return m_host;
        }

        /*!
         * @brief Get the DOTS test host transceiver.
         *
         * @attention Note that it should rarely be required to access the host
         * manually. In particular, publishing instances should always be done
         * via the spoof macros, such as SPOOF_DOTS_PUBLISH().
         *
         * @return const HostTransceiver& A reference to the DOTS test host.
         */
        HostTransceiver& host()
        {
            return m_host;
        }

        /*!
         * @brief Connect a guest transceiver to the DOTS test host.
         *
         * Calling this function connects a guest transceiver to the test
         * environment, by opening a local channel from the guest to the DOTS
         * test host transceiver.
         *
         * The function effectively calls dots::GuestTransceiver::open() with
         * the given arguments.
         *
         * @attention Calling this function manually is only necessary in
         * advanced use cases, when environments with multiple transceivers are
         * required. If possible, it is recommended to always use the global
         * guest transceiver, which is connected automatically.
         *
         * @param guest The guest transceiver to connect to.
         *
         * @param preloadPublishTypes The publish types to preload. This may be
         * empty.
         *
         * @param preloadSubscribeTypes The subscribe types to preload. This
         * may be empty.
         *
         * @param authSecret The authentication secret to use during the
         * handshake. This may be empty.
         */
        void connectGuest(GuestTransceiver& guest, type::DescriptorMap preloadPublishTypes, type::DescriptorMap preloadSubscribeTypes, std::optional<std::string> authSecret)
        {
            guest.open<io::LocalChannel>(std::move(preloadPublishTypes), std::move(preloadSubscribeTypes), std::move(authSecret), m_localListener);
        }

        /*!
         * @brief Connect a guest transceiver to the DOTS test host.
         *
         * Calling this function connects a guest transceiver to the test
         * environment, by opening a local channel from the guest to the DOTS
         * test host transceiver.
         *
         * The function effectively calls dots::GuestTransceiver::open()
         * without arguments.
         *
         * @attention Calling this function manually is only necessary in
         * advanced use cases, when environments with multiple transceivers are
         * required. If possible, it is recommended to always use the global
         * guest transceiver, which is connected automatically.
         */
        void connectGuest(GuestTransceiver& guest)
        {
            guest.open<io::LocalChannel>(m_localListener);
        }

        /*!
         * @brief Get the global DOTS guest transceiver.
         *
         * @attention Using this function should always be preferred to
         * dots::transceiver(), because it implicitly creates and connects the
         * global guest.
         *
         * @return GuestTransceiver& A reference to the DOTS test host.
         */
        GuestTransceiver& globalGuest()
        {
            if (m_globalGuest == nullptr)
            {
                m_globalGuest = &transceiver("dots-global-guest", true);
                connectGuest(*m_globalGuest, io::global_publish_types(), io::global_subscribe_types(), std::optional<std::string>{ std::nullopt });
                processEvents();
            }

            return *m_globalGuest;
        }

        /*!
         * @brief Get the DOTS spoof guest transceiver.
         *
         * @warning This function is intended to be called only by the spoof
         * macros, such as SPOOF_DOTS_PUBLISH(). Using the spoof guest in
         * others ways might result in breaking the test environment.
         *
         * @return GuestTransceiver& A reference to the DOTS spoof guest.
         */
        GuestTransceiver& spoofGuest()
        {
            if (m_spoofGuest == std::nullopt)
            {
                m_spoofGuest.emplace("dots-spoof-guest", m_host.ioContext());
                connectGuest(*m_spoofGuest);
                processEvents();
            }

            return *m_spoofGuest;
        }

        /*!
         * @brief Get the map of mock subscription handlers.
         *
         * @warning This function is intended to be called only by the spoof
         * macros, such as SPOOF_DOTS_PUBLISH(). Accessing the map directly
         * might result in breaking the test environment.
         *
         * @return const std::map<Transceiver*, mock_subscription_handlers_t>&
         * A reference to the map of mock subscription handlers.
         */
        auto mockSubscriptionHandlers() const -> const std::map<Transceiver*, mock_subscription_handlers_t>&
        {
            return m_mockSubscriptionHandlers;
        }

        /*!
         * @brief Get the map of mock subscription handlers.
         *
         * @warning This function is intended to be called only by the spoof
         * macros, such as SPOOF_DOTS_PUBLISH(). Accessing the map directly
         * might result in breaking the test environment.
         *
         * @return const std::map<Transceiver*, mock_subscription_handlers_t>&
         * A reference to the map of mock subscription handlers.
         */
        auto mockSubscriptionHandlers() -> std::map<Transceiver*, mock_subscription_handlers_t>&
        {
            return m_mockSubscriptionHandlers;
        }

        /*!
         * @brief Get mock subscription handler for a specific transceiver and
         * type.
         *
         * @warning This function is intended to be called only by the
         * expectation macros, such as EXPECT_DOTS_PUBLISH_AT_TRANSCEIVER().
         * Accessing a mock subscription handler directly might result in
         * breaking the test environment.
         *
         * @param transceiver The transceiver to get the mock subscription
         * handler for.
         *
         * @param descriptor The type to get the mock subscription handler for.
         *
         * @return  mock_subscription_handler_t& A reference to the mock
         * subscription handler.
         */
        mock_subscription_handler_t& getMockSubscriptionHandler(Transceiver& transceiver, const type::StructDescriptor<>& descriptor)
        {
            auto itTransceiver = m_mockSubscriptionHandlers.try_emplace(&transceiver).first;
            auto [itSubscriptionHandler, emplacedSubscriptionHandler] = itTransceiver->second.try_emplace(&descriptor);

            if (emplacedSubscriptionHandler)
            {
                m_subscriptions.emplace_back(transceiver.subscribe(descriptor, [this, mockHandler = &itSubscriptionHandler->second](const Event<>& event)
                {
                    try
                    {
                        (*mockHandler).AsStdFunction()(event);
                    }
                    catch (const std::exception& e)
                    {
                        LOG_ERROR_S("error in action of publish expectation -> " << e.what());
                    }
                }));
            }

            return itSubscriptionHandler->second;
        }

        /*!
         * @brief Get mock subscription handler for a specific transceiver and
         * type specified by an instance.
         *
         * @warning This function is intended to be called only by the
         * expectation macros, such as EXPECT_DOTS_PUBLISH_AT_TRANSCEIVER().
         * Accessing a mock subscription handler directly might result in
         * breaking the test environment.
         *
         * @tparam T The struct type of the instance. This may be the
         * dots::type::Struct base type.
         *
         * @param transceiver The transceiver to get the mock subscription
         * handler for.
         *
         * @param instance The instance whose type is used to get the mock
         * subscription handler for.
         *
         * @return mock_subscription_handler_t& A reference to the mock
         * subscription handler.
         */
        template <typename T>
        mock_subscription_handler_t& getMockSubscriptionHandler(Transceiver& transceiver, T&& instance)
        {
            constexpr bool IsStruct = std::is_base_of_v<type::Struct, std::decay_t<T>>;
            static_assert(IsStruct, "instance type T has to be a DOTS struct type");

            if constexpr (IsStruct)
            {
                return getMockSubscriptionHandler(transceiver, std::forward<T>(instance)._descriptor());
            }
            else
            {
                auto&& mockSubscriptionHandler = std::declval<mock_subscription_handler_t>();
                return mockSubscriptionHandler;
            }
        }

    private:

        HostTransceiver m_host;
        GuestTransceiver* m_globalGuest;
        std::optional<GuestTransceiver> m_spoofGuest;
        io::LocalListener& m_localListener;
        std::map<Transceiver*, mock_subscription_handlers_t> m_mockSubscriptionHandlers;
        std::vector<Subscription> m_subscriptions;
    };
}

#define IMPL_EXPECT_DOTS_PUBLISH_AT_TRANSCEIVER                                                                                                                            \
[this](dots::Transceiver& transceiver, auto instance, std::optional<dots::types::property_set_t> includedProperties, bool remove, bool isFromMyself) -> auto&              \
{                                                                                                                                                                          \
    return IMPL_EXPECT_DOTS_PUBLISH_AT_SUBSCRIBER(PublishTestBase::getMockSubscriptionHandler(transceiver, instance), instance, includedProperties, remove, isFromMyself); \
}

/*!
 * @brief Create a Google Test expectation for a DOTS publish event at
 * a specific transceiver.
 *
 * This function-like macro creates an expectation that a given
 * instance will be published and result in a corresponding DOTS event
 * at a given transceiver.
 *
 * The expectation will be satisfied if the published (i.e.
 * transmitted) instance of the event compares equal for the given
 * property set (see also dots::testing::EventEqual()).
 *
 * @attention This macro requires usage of the
 * dots::testing::PublishTestBase and will not work if the current
 * Google Test suite is not derived from it.
 *
 * @remark The resulting expectation can either be used individually or
 * as part of a DOTS_EXPECTATION_SEQUENCE().
 *
 * @remark This macro requires events to be create or update events.
 * Expectations for remove events can be created by the
 * EXPECT_DOTS_REMOVE_AT_TRANSCEIVER() macro.
 *
 * @param transceiver The transceiver for which the event is expected
 * to occur.
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
#define EXPECT_DOTS_PUBLISH_AT_TRANSCEIVER                                                                                                     \
[this](dots::Transceiver& transceiver, auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) -> auto& \
{                                                                                                                                              \
    return IMPL_EXPECT_DOTS_PUBLISH_AT_TRANSCEIVER(transceiver, std::forward<decltype(instance)>(instance), includedProperties, false, false); \
}

/*!
 * @brief Create a Google Test expectation for a DOTS remove event at a
 * specific transceiver.
 *
 * This function-like macro creates an expectation that a given
 * instance will be removed and result in a corresponding DOTS event at
 * a given transceiver.
 *
 * The expectation will be satisfied if the published (i.e.
 * transmitted) instance of the event compares equal for the given
 * property set (see also dots::testing::EventEqual()).
 *
 * @attention This macro requires usage of the
 * dots::testing::PublishTestBase and will not work if the current
 * Google Test suite is not derived from it.
 *
 * @remark The resulting expectation can either be used individually or
 * as part of a DOTS_EXPECTATION_SEQUENCE().
 *
 * @remark This macro requires events to be remove events. Expectations
 * for create or update events can be created by the
 * EXPECT_DOTS_PUBLISH_AT_TRANSCEIVER() macro.
 *
 * @param transceiver The transceiver for which the event is expected
 * to occur.
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
#define EXPECT_DOTS_REMOVE_AT_TRANSCEIVER                                                                                                      \
[this](dots::Transceiver& transceiver, auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) -> auto& \
{                                                                                                                                              \
    return IMPL_EXPECT_DOTS_PUBLISH_AT_TRANSCEIVER(transceiver, std::forward<decltype(instance)>(instance), includedProperties, true, false);  \
}

/*!
 * @brief Create a Google Test expectation for a DOTS self publish
 * event at a specific transceiver.
 *
 * This function-like macro works in the same way as
 * EXPECT_DOTS_PUBLISH_AT_TRANSCEIVER() but additionally requires the
 * event to be caused by a publish originating from the given
 * transceiver itself.
 *
 * @attention This macro requires usage of the
 * dots::testing::PublishTestBase and will not work if the current
 * Google Test suite is not derived from it.
 *
 * @remark The resulting expectation can either be used individually or
 * as part of a DOTS_EXPECTATION_SEQUENCE().
 *
 * @remark This macro requires events to be self create or update
 * events. Expectations for self remove events can be created by the
 * EXPECT_DOTS_SELF_REMOVE_AT_TRANSCEIVER() macro.
 *
 * @param transceiver The transceiver for which the event is expected
 * to occur.
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
#define EXPECT_DOTS_SELF_PUBLISH_AT_TRANSCEIVER                                                                                                \
[this](dots::Transceiver& transceiver, auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) -> auto& \
{                                                                                                                                              \
    return IMPL_EXPECT_DOTS_PUBLISH_AT_TRANSCEIVER(transceiver, std::forward<decltype(instance)>(instance), includedProperties, false, true);  \
}

/*!
 * @brief Create a Google Test expectation for a DOTS self remove event
 * at a specific transceiver.
 *
 * This function-like macro works in the same way as
 * EXPECT_DOTS_REMOVE_AT_TRANSCEIVER() but additionally requires the
 * event to be caused by a publish originating from the given
 * transceiver itself.
 *
 * @attention This macro requires usage of the
 * dots::testing::PublishTestBase and will not work if the current
 * Google Test suite is not derived from it.
 *
 * @remark The resulting expectation can either be used individually or
 * as part of a DOTS_EXPECTATION_SEQUENCE().
 *
 * @remark This macro requires events to be self remove events.
 * Expectations for self create or update events can be created by the
 * EXPECT_DOTS_SELF_PUBLISH_AT_TRANSCEIVER() macro.
 *
 * @param transceiver The transceiver for which the event is expected
 * to occur.
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
#define EXPECT_DOTS_SELF_REMOVE_AT_TRANSCEIVER                                                                                                 \
[this](dots::Transceiver& transceiver, auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) -> auto& \
{                                                                                                                                              \
    return IMPL_EXPECT_DOTS_PUBLISH_AT_TRANSCEIVER(transceiver, std::forward<decltype(instance)>(instance), includedProperties, true, true);   \
}

/*!
 * @brief Create a Google Test expectation for a DOTS publish event.
 *
 * This function-like macro creates an expectation that a given
 * instance will be published and result in a corresponding DOTS event.
 *
 * Using the macro is equivalent of using
 * EXPECT_DOTS_PUBLISH_AT_TRANSCEIVER() with the host transceiver of
 * the dots::testing::PublishTestBase. The expectation will therefore
 * be satisfied regardless of where the expected publish originates
 * from.
 *
 * Even though it can be used for arbitrary transceivers that are
 * connected to the test host, it is recommended to only use this macro
 * to test components that are using the global guest transceiver.
 *
 * @attention This macro requires usage of the
 * dots::testing::PublishTestBase and will not work if the current
 * Google Test suite is not derived from it.
 *
 * @remark The resulting expectation can either be used individually or
 * as part of a DOTS_EXPECTATION_SEQUENCE().
 *
 * @remark This macro requires events to be create or update events.
 * Expectations for remove events can be created by the
 * EXPECT_DOTS_REMOVE() macro.
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
#define EXPECT_DOTS_PUBLISH                                                                                                             \
[this](auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) -> auto&                          \
{                                                                                                                                       \
    return EXPECT_DOTS_PUBLISH_AT_TRANSCEIVER(PublishTestBase::host(), std::forward<decltype(instance)>(instance), includedProperties); \
}

/*!
 * @brief Create a Google Test expectation for a DOTS remove event.
 *
 * This function-like macro creates an expectation that a given
 * instance will be removed and result in a corresponding DOTS event.
 *
 * Using the macro is equivalent of using
 * EXPECT_DOTS_REMOVE_AT_TRANSCEIVER() with the host transceiver of the
 * dots::testing::PublishTestBase. The expectation will therefore be
 * satisfied regardless of where the expected remove originates from.
 *
 * Even though it can be used for arbitrary transceivers that are
 * connected to the test host, it is recommended to only use this macro
 * to test components that are using the global guest transceiver.
 *
 * @attention This macro requires usage of the
 * dots::testing::PublishTestBase and will not work if the current
 * Google Test suite is not derived from it.
 *
 * @remark The resulting expectation can either be used individually or
 * as part of a DOTS_EXPECTATION_SEQUENCE().
 *
 * @remark This macro requires events to be remove events. Expectations
 * for create or update events can be created by the
 * EXPECT_DOTS_PUBLISH() macro.
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
#define EXPECT_DOTS_REMOVE                                                                                                             \
[this](auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) -> auto&                         \
{                                                                                                                                      \
    return EXPECT_DOTS_REMOVE_AT_TRANSCEIVER(PublishTestBase::host(), std::forward<decltype(instance)>(instance), includedProperties); \
}

/*!
 * @brief Create a Google Test expectation for a DOTS self publish
 * event.
 *
 * This function-like macro creates an expectation that a given
 * instance will be published by the global guest and that the
 * resulting DOTS event will be processed by the global guest itself.
 *
 * The expectation will therefore only be satisfied if the global guest
 * has a subscription for the given instance type and if the event was
 * caused by a publish originating from the global guest itself.
 *
 * Technically, using the macro is equivalent of using
 * EXPECT_DOTS_PUBLISH_AT_TRANSCEIVER() with the global guest
 * transceiver of the dots::testing::PublishTestBase.
 *
 * @attention This macro requires usage of the
 * dots::testing::PublishTestBase and will not work if the current
 * Google Test suite is not derived from it.
 *
 * @remark Actions following the satisfaction of this expectation are
 * guaranteed to be invoked after the event was processed by the global
 * guest's container.
 *
 * @remark The resulting expectation can either be used individually or
 * as part of a DOTS_EXPECTATION_SEQUENCE().
 *
 * @remark This macro requires events to be create or update events.
 * Expectations for remove events can be created by the
 * EXPECT_DOTS_SELF_REMOVE() macro.
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
#define EXPECT_DOTS_SELF_PUBLISH                                                                                                                    \
[this](auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) -> auto&                                      \
{                                                                                                                                                   \
    return EXPECT_DOTS_SELF_PUBLISH_AT_TRANSCEIVER(PublishTestBase::globalGuest(), std::forward<decltype(instance)>(instance), includedProperties); \
}

/*!
 * @brief Create a Google Test expectation for a DOTS self remove
 * event.
 *
 * This function-like macro creates an expectation that a given
 * instance will be removed by the global guest and that the resulting
 * DOTS event will be processed by the global guest itself.
 *
 * The expectation will therefore only be satisfied if the global guest
 * has a subscription for the given instance type and if the event was
 * caused by a remove originating from the global guest itself.
 *
 * Technically, using the macro is equivalent of using
 * EXPECT_DOTS_SELF_REMOVE_AT_TRANSCEIVER() with the global guest
 * transceiver of the dots::testing::PublishTestBase.
 *
 * @attention This macro requires usage of the
 * dots::testing::PublishTestBase and will not work if the current
 * Google Test suite is not derived from it.
 *
 * @remark Actions following the satisfaction of this expectation are
 * guaranteed to be invoked after the event was processed by the global
 * guest's container.
 *
 * @remark The resulting expectation can either be used individually or
 * as part of a DOTS_EXPECTATION_SEQUENCE().
 *
 * @remark This macro requires events to be self remove events.
 * Expectations for self create or update events can be created by the
 * EXPECT_DOTS_SELF_PUBLISH() macro.
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
#define EXPECT_DOTS_SELF_REMOVE                                                                                                                    \
[this](auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) -> auto&                                     \
{                                                                                                                                                  \
    return EXPECT_DOTS_SELF_REMOVE_AT_TRANSCEIVER(PublishTestBase::globalGuest(), std::forward<decltype(instance)>(instance), includedProperties); \
}

#define IMPL_SPOOF_DOTS_PUBLISH                                                                                                       \
[this](auto instance, std::optional<dots::types::property_set_t> includedProperties, bool remove)                                     \
{                                                                                                                                     \
    constexpr bool IsStruct = std::is_base_of_v<dots::type::Struct, std::decay_t<decltype(instance)>>;                                \
    static_assert(IsStruct, "DOTS publish spoof has to be an instance of a DOTS struct type");                                        \
                                                                                                                                      \
    if constexpr (IsStruct)                                                                                                           \
    {                                                                                                                                 \
        for (auto& [transceiver, mockSubscriptionHandlers] : PublishTestBase::mockSubscriptionHandlers())                             \
        {                                                                                                                             \
            (void)mockSubscriptionHandlers;                                                                                           \
            IMPL_EXPECT_DOTS_PUBLISH_AT_TRANSCEIVER(*transceiver, instance, includedProperties, remove, false).RetiresOnSaturation(); \
        }                                                                                                                             \
        PublishTestBase::spoofGuest().publish(instance, includedProperties, remove);                                                  \
    }                                                                                                                                 \
}

/*!
 * @brief Spoof publish of a specific DOTS instance.
 *
 * This function-like macro spoofs (i.e. "injects" or "simulates") that
 * a given DOTS instance is published by an artificial guest.
 *
 * The resulting publish is for all intents and purposes a "real"
 * publish and will result in corresponding DOTS events for all
 * transceivers that are connected to the host of the
 * dots::testing::PublishTestBase.
 *
 * @attention This macro requires usage of the
 * dots::testing::PublishTestBase and will not work if the current
 * Google Test suite is not derived from it.
 *
 * @attention The publish is performed asynchronously and will not take
 * place until dots::testing::PublishTestBase::processEvents() is
 * called.
 *
 * @remark This macro spoofs instances to be created or updated. The
 * removal of instances can be spoofed with the SPOOF_DOTS_REMOVE()
 * macro.
 *
 * @param instance The instance to spoof.
 *
 * @param includedProperties The property set to include in the spoof.
 * If no set is given, the valid property set of
 * @p instance will be used.
 */
#define SPOOF_DOTS_PUBLISH                                                                            \
[this](auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) \
{                                                                                                     \
    IMPL_SPOOF_DOTS_PUBLISH(std::forward<decltype(instance)>(instance), includedProperties, false);   \
}

/*!
 * @brief Spoof removal of a specific DOTS instance.
 *
 * This function-like macro spoofs (i.e. "injects" or "simulates") that
 * a given DOTS instance is removed by an artificial guest.
 *
 * The resulting remove is for all intents and purposes a "real" remove
 * and will result in corresponding DOTS events for all transceivers
 * that are connected to the host of the
 * dots::testing::PublishTestBase.
 *
 * @attention This macro requires usage of the
 * dots::testing::PublishTestBase and will not work if the current
 * Google Test suite is not derived from it.
 *
 * @attention The publish is performed asynchronously and will not take
 * place until dots::testing::PublishTestBase::processEvents() is
 * called.
 *
 * @remark This macro spoofs instances to be removed. The creation or
 * updating of instances can be spoofed with the SPOOF_DOTS_PUBLISH()
 * macro.
 *
 * @param instance The instance to spoof.
 *
 * @param includedProperties The property set to include in the spoof.
 * If no set is given, the valid property set of
 * @p instance will be used.
 */
#define SPOOF_DOTS_REMOVE                                                                             \
[this](auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) \
{                                                                                                     \
    IMPL_SPOOF_DOTS_PUBLISH(std::forward<decltype(instance)>(instance), includedProperties, true);    \
}

#endif
