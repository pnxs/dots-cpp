#pragma once
#include <type_traits>
#include <memory>
#include <dots/testing/gtest/gtest.h>
#include <dots/testing/gtest/expectations/TransmitExpectation.h>
#include <dots/tools/logging.h>

namespace dots::testing
{
    struct TransmitTestBase : ::testing::Test
    {
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

        ~TransmitTestBase() = default;

        TransmitTestBase& operator = (const TransmitTestBase& rhs) = delete;
        TransmitTestBase& operator = (TransmitTestBase&& rhs) = delete;

    protected:

        static constexpr Connection::id_t UninitializedId = Connection::UninitializedId;
        static constexpr Connection::id_t HostId = Connection::HostId;
        static constexpr Connection::id_t GuestId = Connection::FirstGuestId;
        static constexpr Connection::id_t SpoofId = GuestId + 1;

        static constexpr std::string_view GuestName = "dots-test-guest";
        
        const boost::asio::io_context& ioContext() const
        {
            return m_ioContext;
        }

        boost::asio::io_context& ioContext()
        {
            return m_ioContext;
        }

        const MockChannel& mockChannel() const
        {
            return *m_mockChannel;
        }

        MockChannel& mockChannel()
        {
            return *m_mockChannel;
        }

        const std::string& hostName() const
        {
            return m_hostName;
        }

        void processEvents()
        {
            m_ioContext.poll();
            m_ioContext.restart();
        }

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

    struct transmit_spoof_tag {};

    template <typename T>
    struct TransmitSpoof : transmit_spoof_tag
    {
        TransmitSpoof(uint32_t sender, T instance, std::optional<property_set_t> includedProperties = std::nullopt, bool remove = false) :
            sender(sender),
            instance{ std::move(instance) },
            includedProperties{ includedProperties },
            remove(remove)
        {
            /* do nothing */
        }

        uint32_t sender;
        T instance;
        std::optional<property_set_t> includedProperties = std::nullopt;
        bool remove = false;
    };
}
#define DOTS_EXPECT_TRANSMIT(...) DOTS_EXPECT_TRANSMIT_AT_CHANNEL(TransmitTestBase::mockChannel(), __VA_ARGS__)
#define DOTS_EXPECT_TRANSMIT_SEQUENCE(...) DOTS_EXPECT_TRANSMIT_SEQUENCE_AT_CHANNEL(TransmitTestBase::mockChannel(), __VA_ARGS__)
#define DOTS_EXPECT_NAMED_TRANSMIT_SEQUENCE(sequence_, ...) DOTS_EXPECT_NAMED_TRANSMIT_SEQUENCE_AT_CHANNEL(TransmitTestBase::mockChannel(), sequence_, __VA_ARGS__)

#define DOTS_SPOOF_SINGLE_TRANSMIT(transmitSpoof_)                                                                                                   \
[this](auto&& transmitSpoof)                                                                                                                         \
{                                                                                                                                                    \
    constexpr bool IsStruct = std::is_base_of_v<dots::type::Struct, std::decay_t<decltype(transmitSpoof)>>;                                          \
    constexpr bool IsTransmitSpoof = std::is_base_of_v<dots::testing::transmit_spoof_tag, std::decay_t<decltype(transmitSpoof)>>;                    \
    static_assert(IsStruct || IsTransmitSpoof, "transmit spoof has to be either an instance of a DOTS struct type or of type TransmitExpectation");  \
                                                                                                                                                     \
    if constexpr (IsStruct)                                                                                                                          \
    {                                                                                                                                                \
        TransmitTestBase::mockChannel().spoof(TransmitTestBase::SpoofId, transmitSpoof, std::nullopt, false);                                        \
    }                                                                                                                                                \
    else if constexpr (IsTransmitSpoof)                                                                                                              \
    {                                                                                                                                                \
        TransmitTestBase::mockChannel().spoof(transmitSpoof.sender, transmitSpoof.instance, transmitSpoof.includedProperties, transmitSpoof.remove); \
    }                                                                                                                                                \
}(transmitSpoof_)

#define DOTS_SPOOF_TRANSMIT(...)                                                               \
[this](auto&&... transmitSpoofs)                                                               \
{                                                                                              \
    (DOTS_SPOOF_SINGLE_TRANSMIT(std::forward<decltype(transmitSpoofs)>(transmitSpoofs)), ...); \
    TransmitTestBase::processEvents();                                                         \
}(__VA_ARGS__)

#define DOTS_SPOOF_TRANSMIT_REMOVE(...)                                                                                           \
[this](auto&&... instances)                                                                                                       \
{                                                                                                                                 \
    constexpr bool AreStructs = std::conjunction_v<std::is_base_of<dots::type::Struct, std::decay_t<decltype(instances)>>...>;    \
    static_assert(AreStructs, "arguments to spoof remove have to be instances of DOTS struct types");                             \
                                                                                                                                  \
    if constexpr (AreStructs)                                                                                                     \
    {                                                                                                                             \
        DOTS_SPOOF_TRANSMIT(dots::testing::TransmitSpoof{ std::forward<decltype(instances)>(instances), std::nullopt, true }...); \
    }                                                                                                                             \
}(__VA_ARGS__)
