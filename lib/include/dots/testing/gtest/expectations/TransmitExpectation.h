#pragma once
#include <boost/asio.hpp>
#include <dots/testing/gtest/gtest.h>
#include <dots/io/Channel.h>
#include <dots/Connection.h>
#include <dots/testing/gtest/expectations/CallExpectation.h>
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

    struct transmit_expectation_tag {};

    template <typename T>
    struct TransmitExpectation : transmit_expectation_tag
    {
        TransmitExpectation(T instance, std::optional<dots::property_set_t> includedProperties = std::nullopt, bool remove = false) :
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

#define DOTS_MAKE_EXPECT_TRANSMIT_AT_CHANNEL(mockChannel_)                                                                                                                  \
[&](auto&&... transmitExpectations) -> decltype(auto)                                                                                                                       \
{                                                                                                                                                                           \
    auto expect_transmit_at_channel = [&](auto&& transmitExpectation) -> decltype(auto)                                                                                     \
    {                                                                                                                                                                       \
        constexpr bool IsStruct = std::is_base_of_v<dots::type::Struct, std::decay_t<decltype(transmitExpectation)>>;                                                       \
        constexpr bool IsTransmitExpectation = std::is_base_of_v<dots::testing::transmit_expectation_tag, std::decay_t<decltype(transmitExpectation)>>;                     \
        static_assert(IsStruct || IsTransmitExpectation, "transmit expectation has to be either an instance of a DOTS struct type or of type TransmitExpectation");         \
                                                                                                                                                                            \
        if constexpr (IsStruct)                                                                                                                                             \
        {                                                                                                                                                                   \
            const auto& instance = transmitExpectation;                                                                                                                     \
            return EXPECT_CALL((mockChannel_).transmitMock(), Call(dots::testing::TransmissionEqual(instance, std::nullopt, false)));                                                \
        }                                                                                                                                                                   \
        else if constexpr (IsTransmitExpectation)                                                                                                                           \
        {                                                                                                                                                                   \
            const auto& instance = transmitExpectation.instance;                                                                                                            \
            return EXPECT_CALL((mockChannel_).transmitMock(), Call(dots::testing::TransmissionEqual(instance, transmitExpectation.includedProperties, transmitExpectation.remove))); \
        }                                                                                                                                                                   \
        else                                                                                                                                                                \
        {                                                                                                                                                                   \
            auto&& instance = std::declval<dots::type::Struct>();                                                                                                           \
            return EXPECT_CALL(std::declval<dots::testing::MockChannel>().transmitMock(), Call(dots::testing::TransmissionEqual(instance, std::nullopt, false)));                  \
        }                                                                                                                                                                   \
   };                                                                                                                                                                       \
                                                                                                                                                                            \
    if constexpr (sizeof...(transmitExpectations) == 1)                                                                                                                     \
    {                                                                                                                                                                       \
        return (expect_transmit_at_channel(std::forward<decltype(transmitExpectations)>(transmitExpectations)), ...);                                                       \
    }                                                                                                                                                                       \
    else                                                                                                                                                                    \
    {                                                                                                                                                                       \
        return std::tie(expect_transmit_at_channel(std::forward<decltype(transmitExpectations)>(transmitExpectations))...);                                                 \
    }                                                                                                                                                                       \
}

#define DOTS_EXPECT_TRANSMIT_AT_CHANNEL(mockChannel_, transmitExpectation_) DOTS_MAKE_EXPECT_TRANSMIT_AT_CHANNEL(mockChannel_)(transmitExpectation_)
#define DOTS_EXPECT_TRANSMIT_SEQUENCE_AT_CHANNEL(mockChannel_, ...) DOTS_EXPECT_CONSECUTIVE_CALL_SEQUENCE(DOTS_MAKE_EXPECT_TRANSMIT_AT_CHANNEL(mockChannel_), __VA_ARGS__)
#define DOTS_EXPECT_NAMED_TRANSMIT_SEQUENCE_AT_CHANNEL(mockChannel_, sequence_, ...) DOTS_EXPECT_NAMED_CALL_SEQUENCE(DOTS_MAKE_EXPECT_TRANSMIT_AT_CHANNEL(mockChannel_), sequence_, __VA_ARGS__)
