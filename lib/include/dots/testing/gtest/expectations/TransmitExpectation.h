#pragma once
#include <boost/asio.hpp>
#include <dots/testing/gtest/gtest.h>
#include <dots/io/Channel.h>
#include <dots/Connection.h>
#include <dots/testing/gtest/matchers/TransmissionMatcher.h>

namespace dots::testing
{
    struct MockChannel : io::Channel
    {
        using transmit_mock_t = ::testing::MockFunction<void(const io::Transmission&)>;

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

        const boost::asio::io_context& ioContext() const
        {
            return m_ioContext;
        }
        boost::asio::io_context& ioContext()
        {
            return m_ioContext;
        }

        const transmit_mock_t& transmitMock() const
        {
            return m_transmitMock;
        }

        transmit_mock_t& transmitMock()
        {
            return m_transmitMock;
        }

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

#define EXPECT_DOTS_TRANSMIT_AT_CHANNEL                                                                                                             \
[](dots::testing::MockChannel& mockChannel, auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) -> auto& \
{                                                                                                                                                   \
    return IMPL_EXPECT_DOTS_TRANSMIT_AT_CHANNEL(mockChannel, std::forward<decltype(instance)>(instance), includedProperties, false);                \
}

#define EXPECT_DOTS_REMOVE_TRANSMIT_AT_CHANNEL                                                                                                      \
[](dots::testing::MockChannel& mockChannel, auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) -> auto& \
{                                                                                                                                                   \
    return IMPL_EXPECT_DOTS_TRANSMIT_AT_CHANNEL(mockChannel, std::forward<decltype(instance)>(instance), includedProperties, true);                 \
}
