#pragma once
#include <type_traits>
#include <memory>
#include <dots/testing/gtest/gtest.h>
#include <dots/testing/gtest/expectations/ExpectationSequence.h>
#include <dots/testing/gtest/expectations/TransmitExpectation.h>
#include <dots/tools/logging.h>

namespace dots::testing
{
    /*!
     * @class TransmitTestBase TransmitTestBase.h
     * <dots/testing/gtest/TransmitTestBase.h>
     *
     * @brief Base class for Google Test suites that test components based on
     * DOTS channels.
     *
     * Deriving from this class provides Google Test support for components
     * that rely on DOTS transmissions via channels (see dots::io::Channel).
     *
     * Expectations and spoofs of DOTS transmissions can be created via
     * corresponding test macros (e.g. EXPECT_DOTS_TRANSMIT() and
     * SPOOF_DOTS_TRANSMIT() respectively).
     *
     * @attention Unless otherwise required, it is recommended to write test
     * cases in the "sequence style" by utilizing the
     * DOTS_EXPECTATION_SEQUENCE() macro.
     *
     * @warning This class is intended to be used as a base class only.
     */
    struct TransmitTestBase : ::testing::Test
    {
        /*!
         * @brief Set up the DOTS test environment.
         *
         * Calling this constructor from a derived class initializes the
         * DOTS test environment.
         *
         * Similar to an "actual" DOTS application, the test environment is
         * running asynchronously in an event loop. This is because
         * components based on DOTS are inherently asynchronous and also
         * often rely on other asynchronous mechanisms, such as posts or
         * timer events.
         *
         * The event loop defaults to the global event loop, but can be
         * provided by the user if necessary.
         *
         * After the constructor has returned, the test environment will
         * have been initialized and be ready for use.
         *
         * @warning The given IO context must match the IO context used by
         * the components that are part of the test.
         *
         * @remark To reduce clutter in the test output, the constructor
         * sets the logging level to tools::Level::warn. This can be
         * overridden by setting the LOGGING_LEVEL environment variable.
         *
         * @param ioContext The ASIO IO context (i.e. the "event loop") to
         * use.
         *
         * @param hostName The name the host will use to identify itself.
         */
        TransmitTestBase(boost::asio::io_context& ioContext = io::global_io_context(), std::string hostName = "dots-test-host") :
            m_ioContext(ioContext),
            m_mockChannel{ dots::io::make_channel<MockChannel>(ioContext) },
            m_hostName{ std::move(hostName) }
        {
            m_ioContext.restart();

            // disable verbose logging unless overriden by the user
            if (::getenv("LOGGING_LEVEL") == nullptr)
            {
                tools::loggingFrontend().setLogLevel(tools::Level::warn);
            }
        }

        TransmitTestBase(const TransmitTestBase& other) = delete;
        TransmitTestBase(TransmitTestBase&& other) = delete;

        /*!
         * @brief Tear down the DOTS test environment.
         *
         * Calling the destructor from a derived class cleans up the test
         * environment. This includes the teardown of the mock channel.
         */
        ~TransmitTestBase() = default;

        TransmitTestBase& operator = (const TransmitTestBase& rhs) = delete;
        TransmitTestBase& operator = (TransmitTestBase&& rhs) = delete;

    protected:

        static constexpr Connection::id_t UninitializedId = Connection::UninitializedId;
        static constexpr Connection::id_t HostId = Connection::HostId;
        static constexpr Connection::id_t GuestId = Connection::FirstGuestId;
        static constexpr Connection::id_t SpoofId = GuestId + 1;

        static constexpr std::string_view GuestName = "dots-test-guest";
        
        /*!
         * @brief Get the currently used IO context.
         *
         * Note that this is the same IO context that was given in
         * TransmitTestBase().
         *
         * @return const boost::asio::io_context& A reference to the
         * currently used IO context.
         */
        const boost::asio::io_context& ioContext() const
        {
            return m_ioContext;
        }

        /*!
         * @brief Get the currently used IO context.
         *
         * Note that this is the same IO context that was given in
         * TransmitTestBase().
         *
         * @return boost::asio::io_context& A reference to the currently
         * used IO context.
         */
        boost::asio::io_context& ioContext()
        {
            return m_ioContext;
        }

        /*!
         * @brief Get the mock channel.
         *
         * @attention Note that it should rarely be required to access the
         * mock channel manually. In particular, creating expectations and
         * spoofing transmits should be done via the corresponding macros,
         * such as EXPECT_DOTS_TRANSMIT() and SPOOF_DOTS_TRANSMIT().
         *
         * @return MockChannel& A reference to the mock channel.
         */
        const MockChannel& mockChannel() const
        {
            return *m_mockChannel;
        }

        /*!
         * @brief Get the mock channel.
         *
         * Note that this is the same IO context that was given in
         * PublishTestBase().
         *
         * @attention It should rarely be required to access the mock
         * channel manually. In particular, creating expectations and
         * spoofing transmits should be done via the corresponding macros,
         * such as EXPECT_DOTS_TRANSMIT() and SPOOF_DOTS_TRANSMIT().
         *
         * @return MockChannel& A reference to the mock channel.
         */
        MockChannel& mockChannel()
        {
            return *m_mockChannel;
        }

        /*!
         * @brief Get the host name.
         *
         * Note that this is the same name that was given in
         * PublishTestBase().
         *
         * @return const std::string& A reference to the host name.
         */
        const std::string& hostName() const
        {
            return m_hostName;
        }

        /*!
         * @brief Process all outstanding events.
         *
         * Calling this function will process all outstanding events (i.e.
         * ready handlers) in the event loop (i.e. the IO context), until
         * it has been stopped or there are no more ready handlers.
         *
         * This function will never block and in particular not wait for
         * any active timers to end.
         *
         * Technically, this effectively calls
         * boost::asio::io_context::poll() followed by a
         * boost::asio::io_context::restart().
         *
         */
        void processEvents()
        {
            m_ioContext.poll();
            m_ioContext.restart();
        }

        /*!
         * @brief Create unique guest id.
         *
         * @return Connection::id_t The unique guest id.
         */
        static Connection::id_t MakeGuestId()
        {
            return M_nextGuestId++;
        }

    private:

        inline static Connection::id_t M_nextGuestId = SpoofId + 1;

        boost::asio::io_context& m_ioContext;
        std::shared_ptr<MockChannel> m_mockChannel;
        std::string m_hostName;
    };
}

#define IMPL_EXPECT_DOTS_TRANSMIT                                                                                                                                        \
[this](auto&& instance, std::optional<dots::types::property_set_t> includedProperties, bool remove) -> auto&                                                             \
{                                                                                                                                                                        \
    return IMPL_EXPECT_DOTS_TRANSMIT_AT_CHANNEL(dots::testing::TransmitTestBase::mockChannel(), std::forward<decltype(instance)>(instance), includedProperties, remove); \
}

/*!
 * @brief Create a Google Test expectation for a DOTS transmission.
 *
 * This function-like macro creates an expectation that a given
 * instance will be transmitted via the mock channel of the
 * dots::testing::TransmitTestBase.
 *
 * Using the macro is equivalent of using
 * EXPECT_DOTS_TRANSMIT_AT_CHANNEL() with the mock channel of the
 * dots::testing::TransmitTestBase.
 *
 * The expectation will be satisfied if the transmitted instance
 * compares equal for the given property set (see also
 * dots::testing::TransmissionEqual()).
 *
 * @attention This macro requires usage of the
 * dots::testing::TransmitTestBase and will not work if the current
 * Google Test suite is not derived from it.
 *
 * @remark The resulting expectation can either be used individually or
 * as part of a DOTS_EXPECTATION_SEQUENCE().
 *
 * @remark This macro requires transmissions to not have the remove
 * flag set. Expectations for remove transmissions can be created by
 * the EXPECT_DOTS_REMOVE_TRANSMIT() macro.
 *
 * @param instance The instance to compare the transmitted instance to.
 *
 * @param includedProperties The property set to include in the
 * equality comparison. If no set is given, the valid property set of
 * @p instance will be used.
 *
 * @return auto& A reference to the created Google Test expectation.
 */
#define EXPECT_DOTS_TRANSMIT                                                                                   \
[this](auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) -> auto& \
{                                                                                                              \
    return IMPL_EXPECT_DOTS_TRANSMIT(std::forward<decltype(instance)>(instance), includedProperties, false);   \
}

/*!
 * @brief Create a Google Test expectation for a DOTS remove
 * transmission.
 *
 * This function-like macro creates an expectation that a given
 * instance will be transmitted as removed via the mock channel of the
 * dots::testing::TransmitTestBase.
 *
 * Using the macro is equivalent of using
 * EXPECT_DOTS_REMOVE_TRANSMIT_AT_CHANNEL() with the mock channel of
 * the dots::testing::TransmitTestBase.
 *
 * The expectation will be satisfied if the transmitted instance
 * compares equal for the given property set (see also
 * dots::testing::TransmissionEqual()).
 *
 * @attention This macro requires usage of the
 * dots::testing::TransmitTestBase and will not work if the current
 * Google Test suite is not derived from it.
 *
 * @remark The resulting expectation can either be used individually or
 * as part of a DOTS_EXPECTATION_SEQUENCE().
 *
 * @remark This macro requires transmissions to have the remove flag
 * set. Expectations for non-remove transmissions can be created by the
 * EXPECT_DOTS_TRANSMIT() macro.
 *
 * @param instance The instance to compare the transmitted instance to.
 *
 * @param includedProperties The property set to include in the
 * equality comparison. If no set is given, the valid property set of
 * @p instance will be used.
 *
 * @return auto& A reference to the created Google Test expectation.
 */
#define EXPECT_DOTS_REMOVE_TRANSMIT                                                                            \
[this](auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) -> auto& \
{                                                                                                              \
    return IMPL_EXPECT_DOTS_TRANSMIT(std::forward<decltype(instance)>(instance), includedProperties, true);    \
}

#define IMPL_SPOOF_DOTS_TRANSMIT                                                                                                                 \
[this](auto&& instance, std::optional<dots::types::property_set_t> includedProperties, bool remove, uint32_t sender = TransmitTestBase::SpoofId) \
{                                                                                                                                                \
    constexpr bool IsStruct = std::is_base_of_v<dots::type::Struct, std::decay_t<decltype(instance)>>;                                           \
    static_assert(IsStruct, "DOTS transmit spoof has to be an instance of a DOTS struct type");                                                  \
                                                                                                                                                 \
    if constexpr (IsStruct)                                                                                                                      \
    {                                                                                                                                            \
        TransmitTestBase::mockChannel().spoof(sender, std::forward<decltype(instance)>(instance), includedProperties, remove);                   \
    }                                                                                                                                            \
}

/*!
 * @brief Spoof transmit of a specific DOTS instance.
 *
 * This function-like macro spoofs (i.e. "injects" or "simulates") that
 * a given DOTS instance is transmitted by an artificial peer.
 *
 * The resulting transmit is for all intents and purposes a "real"
 * transmit and will result in corresponding handler invocations for
 * the user of the channel.
 *
 * @attention This macro requires usage of the
 * dots::testing::TransmitTestBase and will not work if the current
 * Google Test suite is not derived from it.
 *
 * @attention The transmit is performed asynchronously and will not
 * take place until dots::testing::TransmitTestBase::processEvents() is
 * called.
 *
 * @remark This macro spoofs instances without the remove flag being
 * set. The removal of instances can be spoofed with the
 * SPOOF_DOTS_REMOVE_TRANSMIT() macro.
 *
 * @param instance The instance to spoof.
 *
 * @param includedProperties The property set to include in the spoof.
 * If no set is given, the valid property set of
 * @p instance will be used.
 */
#define SPOOF_DOTS_TRANSMIT                                                                           \
[this](auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) \
{                                                                                                     \
    IMPL_SPOOF_DOTS_TRANSMIT(std::forward<decltype(instance)>(instance), includedProperties, false);  \
}

/*!
 * @brief Spoof transmit of a specific DOTS instance from a specific
 * sender.
 *
 * This function-like macro works in a similar way as
 * SPOOF_DOTS_TRANSMIT() but uses the given sender id in the header.
 *
 * @attention This macro requires usage of the
 * dots::testing::TransmitTestBase and will not work if the current
 * Google Test suite is not derived from it.
 *
 * @attention The transmit is performed asynchronously and will not
 * take place until dots::testing::TransmitTestBase::processEvents() is
 * called.
 *
 * @param sender The sender id that will be used in the header.
 *
 * @param instance The instance to spoof.
 *
 * @param includedProperties The property set to include in the spoof.
 * If no set is given, the valid property set of
 * @p instance will be used.
 *
 * @param remove Specifies whether the remove flag in the header will
 * be set.
 */
#define SPOOF_DOTS_TRANSMIT_FROM_SENDER                                                                                                     \
[this](uint32_t sender, auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt, bool remove = false) \
{                                                                                                                                           \
    IMPL_SPOOF_DOTS_TRANSMIT(std::forward<decltype(instance)>(instance), includedProperties, remove, sender);                               \
}

/*!
 * @brief Spoof remove transmit of a specific DOTS instance.
 *
 * This function-like macro spoofs (i.e. "injects" or "simulates") that
 * a given DOTS instance is transmitted as removed by an artificial
 * peer.
 *
 * The resulting transmit is for all intents and purposes a "real"
 * transmit and will result in corresponding handler invocations for
 * the user of the channel.
 *
 * @attention This macro requires usage of the
 * dots::testing::TransmitTestBase and will not work if the current
 * Google Test suite is not derived from it.
 *
 * @attention The transmit is performed asynchronously and will not
 * take place until dots::testing::TransmitTestBase::processEvents() is
 * called.
 *
 * @remark This macro spoofs instances with the remove flag being set.
 * Transmissions without remove can be spoofed with the
 * SPOOF_DOTS_TRANSMIT() macro.
 *
 * @param instance The instance to spoof.
 *
 * @param includedProperties The property set to include in the spoof.
 * If no set is given, the valid property set of
 * @p instance will be used.
 */
#define SPOOF_DOTS_REMOVE_TRANSMIT                                                                    \
[this](auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) \
{                                                                                                     \
    IMPL_SPOOF_DOTS_TRANSMIT(std::forward<decltype(instance)>(instance), includedProperties, true);   \
}
