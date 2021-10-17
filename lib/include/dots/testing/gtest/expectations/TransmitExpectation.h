#pragma once
#include <boost/asio.hpp>
#include <dots/testing/gtest/gtest.h>
#include <dots/io/Channel.h>
#include <dots/Connection.h>
#include <dots/testing/gtest/matchers/TransmissionMatcher.h>

namespace dots::testing
{
    /*!
     * @class MockChannel TransmitExpectation.h <dots/testing/gtest/TransmitExpectation.h>
     *
     * @brief Mock for DOTS IO channels.
     *
     * This channel can be used as a mock to test components that rely on DOTS
     * IO channels.
     *
     * Note that this is not a classical Google Test mock that directly mocks
     * virtual functions. Instead, the actual mocking is realized via a
     * separate mock function, which is invoked asynchronously.
     *
     * @attention Even though it is possible to manually access the mock, it is
     * recommended to use the dots::testing::TransmitTestBase, along with the
     * corresponding test macros whenever possible.
     */
    struct MockChannel : io::Channel
    {
        using transmit_mock_t = ::testing::MockFunction<void(const io::Transmission&)>;

        /*!
         * @brief Construct a new Mock Channel object.
         *
         * Similar to an "actual" DOTS IO channel, the mock is running
         * asynchronously in an event loop. This is because components
         * based on DOTS are inherently asynchronous and also often rely on
         * other asynchronous mechanisms, such as posts or timer events.
         *
         * The event loop defaults to the global event loop, but can be
         * provided by the user if necessary.
         *
         * @remark As with all channels, the mock channel can only be
         * constructed managed by a std::shared_ptr via the
         * dots::io::make_channel() factory function.
         *
         * @param key The construction key.
         *
         * @param ioContext The ASIO IO context (i.e. the "event loop") to
         * use.
         */
        MockChannel(Channel::key_t key, boost::asio::io_context& ioContext) :
            Channel(key),
            m_ioContext{ std::ref(ioContext) }
        {
            initEndpoints(io::Endpoint{ "mock:/" }, io::Endpoint{ "mock:/" });
        }
        MockChannel(const MockChannel& other) = delete;
        MockChannel(MockChannel&& other) = delete;
        virtual ~MockChannel() = default;

        MockChannel& operator = (const MockChannel& rhs) = delete;
        MockChannel& operator = (MockChannel&& rhs) = delete;

        void transmitImpl(const DotsHeader& header, const type::Struct& instance) override
        {
            transmitImpl(io::Transmission{ header, type::AnyStruct{ instance } });
        }

        void transmitImpl(const io::Transmission& transmission) override
        {
            boost::asio::post(m_ioContext.get(), [this, transmission = io::Transmission{ transmission.header(), transmission.instance() }]()
            {
                m_transmitMock.AsStdFunction()(transmission);
            });
        }

        /*!
         * @brief Get the currently used IO context.
         *
         * Note that this is the same IO context that was given in
         * MockChannel().
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
         * MockChannel().
         *
         * @return const boost::asio::io_context& A reference to the
         * currently used IO context.
         */
        boost::asio::io_context& ioContext()
        {
            return m_ioContext;
        }

        /*!
         * @brief Get the underlying transmit mock function.
         *
         * @attention Even though it is possible to manually create
         * expectations on the mock, it is recommended to always use the
         * corresponding test macros, such
         * EXPECT_DOTS_TRANSMIT_AT_CHANNEL(), instead.
         *
         * @return const transmit_mock_t& A reference to the transmit mock
         * function.
         */
        const transmit_mock_t& transmitMock() const
        {
            return m_transmitMock;
        }

        /*!
         * @brief Get the underlying transmit mock function.
         *
         * @attention Even though it is possible to manually create
         * expectations on the mock, it is recommended to always use the
         * corresponding test macros, such
         * EXPECT_DOTS_TRANSMIT_AT_CHANNEL(), instead.
         *
         * @return const transmit_mock_t& A reference to the transmit mock
         * function.
         */
        transmit_mock_t& transmitMock()
        {
            return m_transmitMock;
        }

        /*!
         * @brief Spoof transmit of a specific DOTS instance.
         *
         * This function spoofs (i.e. "injects" or "simulates") that a
         * given DOTS instance is transmitted by an artificial peer.
         *
         * The resulting transmit is for all intents and purposes a "real"
         * transmit and will result in corresponding handler invocations
         * for the user of the channel.
         *
         * @attention The transmit is performed asynchronously and will not
         * take place until being processed by the event loop.
         *
         * @remark This version of the function requires manual
         * construction of the header. A default header can be constructed
         * automatically by another overload.
         *
         * @param header The header to spoof.
         *
         * @param instance The instance to spoof.
         */
        void spoof(const DotsHeader& header, const type::Struct& instance)
        {
            boost::asio::post(m_ioContext.get(), [this, this_{ weak_from_this() }, header = header, instance = type::AnyStruct{ instance }]() mutable
            {
                try
                {
                    if (this_.expired())
                    {
                        return;
                    }

                    processReceive(io::Transmission{ std::move(header), std::move(instance) });
                }
                catch (...)
                {
                    processError(std::current_exception());
                }
            });
        }

        /*!
         * Spoof transmit of a specific DOTS instance from a specific
         * sender.
         *
         * This function spoofs (i.e. "injects" or "simulates") that a
         * given DOTS instance is transmitted by a specific peer.
         *
         * The resulting transmit is for all intents and purposes a "real"
         * transmit and will result in corresponding handler invocations
         * for the user of the channel.
         *
         * @attention The transmit is performed asynchronously and will not
         * take place until being processed by the event loop.
         *
         * @remark This version of the function automatically constructs
         * the header. Manual construction of the header is supported by
         * another overload.
         *
         * @param sender The sender id that will be used in the header.
         *
         * @param instance The instance to spoof.
         *
         * @param includedProperties The property set to include in the
         * spoof. If no set is given, the valid property set of
         * @p instance will be used.
         *
         * @param remove Specifies whether the remove flag in the header
         * will be set.
         */
        void spoof(uint32_t sender, const type::Struct& instance, std::optional<types::property_set_t> includedProperties = std::nullopt, bool remove = false)
        {
            if (includedProperties == std::nullopt)
            {
                includedProperties = instance._validProperties();
            }
            else
            {
                *includedProperties ^= instance._descriptor().properties();
            }

            spoof(DotsHeader{
                DotsHeader::typeName_i{ instance._descriptor().name() },
                DotsHeader::sentTime_i{ types::timepoint_t::Now() },
                DotsHeader::attributes_i{ *includedProperties },
                DotsHeader::sender_i{ sender },
                DotsHeader::removeObj_i{ remove }
            }, instance);
        }

        /*!
         * @brief Spoof transmit of a specific DOTS instance.
         *
         * This function spoofs (i.e. "injects" or "simulates") that a
         * given DOTS instance is transmitted by an artificial peer.
         *
         * The resulting transmit is for all intents and purposes a "real"
         * transmit and will result in corresponding handler invocations
         * for the user of the channel.
         *
         * @attention The transmit is performed asynchronously and will not
         * take place until being processed by the event loop.
         *
         * @remark This version of the function automatically constructs
         * the header. Manual construction of the header is supported by
         * another overload.
         *
         * @param instance The instance to spoof.
         *
         * @param includedProperties The property set to include in the
         * spoof. If no set is given, the valid property set of
         * @p instance will be used.
         *
         * @param remove Specifies whether the remove flag in the header
         * will be set.
         */
        void spoof(const type::Struct& instance, std::optional<types::property_set_t> includedProperties = std::nullopt, bool remove = false)
        {
            spoof(Connection::HostId, instance, includedProperties, remove);
        }

    protected:

        void asyncReceiveImpl() override
        {
            /* do nothing */
        }

    private:

        std::reference_wrapper<boost::asio::io_context> m_ioContext;
        transmit_mock_t m_transmitMock;
    };
}

#define IMPL_EXPECT_DOTS_TRANSMIT_AT_CHANNEL                                                                                                                        \
[](dots::testing::MockChannel& mockChannel, auto&& instance, std::optional<dots::types::property_set_t> includedProperties, bool remove) -> auto&                   \
{                                                                                                                                                                   \
    return EXPECT_CALL(mockChannel.transmitMock(), Call(dots::testing::TransmissionEqual(std::forward<decltype(instance)>(instance), includedProperties, remove))); \
}

/*!
 * @brief Create a Google Test expectation for a DOTS transmit at a
 * specific dots::testing::MockChannel.
 *
 * This function-like macro creates an expectation that a given
 * instance will be transmitted via a given mock channel.
 *
 * The expectation will be satisfied if the transmitted instance
 * compares equal for the given property set (see also
 * dots::testing::TransmissionEqual()).
 *
 * @remark This macro is usually used indirectly by higher-level macros
 * such as EXPECT_DOTS_TRANSMIT().
 *
 * @remark The resulting expectation can either be used individually or
 * as part of a DOTS_EXPECTATION_SEQUENCE().
 *
 * @remark This macro requires transmissions to not have the remove
 * flag set. Expectations for remove transmissions can be created by
 * the EXPECT_DOTS_REMOVE_TRANSMIT_AT_CHANNEL() macro.
 *
 * @param mockChannel The mock channel on which the transmission is
 * expected to occur.
 *
 * @param instance The instance to compare the transmitted instance to.
 *
 * @param includedProperties The property set to include in the
 * equality comparison. If no set is given, the valid property set of
 * @p instance will be used.
 *
 * @return auto& A reference to the created Google Test expectation.
 */
#define EXPECT_DOTS_TRANSMIT_AT_CHANNEL                                                                                                             \
[](dots::testing::MockChannel& mockChannel, auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) -> auto& \
{                                                                                                                                                   \
    return IMPL_EXPECT_DOTS_TRANSMIT_AT_CHANNEL(mockChannel, std::forward<decltype(instance)>(instance), includedProperties, false);                \
}

/*!
 * @brief Create a Google Test expectation for a DOTS remove transmit
 * at a specific dots::testing::MockChannel.
 *
 * This function-like macro creates an expectation that a given
 * instance will be transmitted as removed via a given mock channel.
 *
 * The expectation will be satisfied if the transmitted instance
 * compares equal for the given property set (see also
 * dots::testing::TransmissionEqual()).
 *
 * @remark This macro is usually used indirectly by higher-level macros
 * such as EXPECT_DOTS_REMOVE_TRANSMIT().
 *
 * @remark The resulting expectation can either be used individually or
 * as part of a DOTS_EXPECTATION_SEQUENCE().
 *
 * @remark This macro requires transmissions to have the remove flag
 * set. Expectations for non-remove transmissions can be created by the
 * EXPECT_DOTS_TRANSMIT_AT_CHANNEL() macro.
 *
 * @param mockChannel The mock channel on which the transmission is
 * expected to occur.
 *
 * @param instance The instance to compare the transmitted instance to.
 *
 * @param includedProperties The property set to include in the
 * equality comparison. If no set is given, the valid property set of
 * @p instance will be used.
 *
 * @return auto& A reference to the created Google Test expectation.
 */
#define EXPECT_DOTS_REMOVE_TRANSMIT_AT_CHANNEL                                                                                                      \
[](dots::testing::MockChannel& mockChannel, auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) -> auto& \
{                                                                                                                                                   \
    return IMPL_EXPECT_DOTS_TRANSMIT_AT_CHANNEL(mockChannel, std::forward<decltype(instance)>(instance), includedProperties, true);                 \
}
