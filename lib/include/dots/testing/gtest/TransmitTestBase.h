#pragma once
#include <type_traits>
#include <memory>
#include <dots/testing/gtest/gtest.h>
#include <dots/testing/gtest/expectations/ExpectationSequence.h>
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
}

#define IMPL_EXPECT_DOTS_TRANSMIT                                                                                                                                        \
[this](auto&& instance, std::optional<dots::types::property_set_t> includedProperties, bool remove) -> auto&                                                             \
{                                                                                                                                                                        \
    return IMPL_EXPECT_DOTS_TRANSMIT_AT_CHANNEL(dots::testing::TransmitTestBase::mockChannel(), std::forward<decltype(instance)>(instance), includedProperties, remove); \
}

#define EXPECT_DOTS_TRANSMIT                                                                                   \
[this](auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) -> auto& \
{                                                                                                              \
    return IMPL_EXPECT_DOTS_TRANSMIT(std::forward<decltype(instance)>(instance), includedProperties, false);   \
}

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

#define SPOOF_DOTS_TRANSMIT                                                                           \
[this](auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) \
{                                                                                                     \
    IMPL_SPOOF_DOTS_TRANSMIT(std::forward<decltype(instance)>(instance), includedProperties, false);  \
}

#define SPOOF_DOTS_TRANSMIT_FROM_SENDER                                                                                                     \
[this](uint32_t sender, auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt, bool remove = false) \
{                                                                                                                                           \
    IMPL_SPOOF_DOTS_TRANSMIT(std::forward<decltype(instance)>(instance), includedProperties, remove, sender);                               \
}

#define SPOOF_DOTS_REMOVE_TRANSMIT                                                                    \
[this](auto&& instance, std::optional<dots::types::property_set_t> includedProperties = std::nullopt) \
{                                                                                                     \
    IMPL_SPOOF_DOTS_TRANSMIT(std::forward<decltype(instance)>(instance), includedProperties, true);   \
}
