#include <sstream>
#include <optional>
#include <dots/Connection.h>
#include <dots/io/Io.h>
#include <dots/type/Registry.h>
#include <dots/testing/gtest/expectations/TransmitExpectation.h>
#include <dots/testing/gtest/TransmitTestBase.h>
#include <DotsTestStruct.dots.h>

struct TestConnectionBase : dots::testing::TransmitTestBase
{
    TestConnectionBase(bool host) :
        m_sut{ std::in_place, mockChannel().shared_from_this(), host }
    {
        /* do nothing */
    }
    
    ~TestConnectionBase()
    {
        if (m_sut->state() == DotsConnectionState::connected)
        {
            // close gracefully on destruction
            DOTS_EXPECT_TRANSMIT(DotsMsgError{
                DotsMsgError::errorCode_i{ 0 }
            });
        }

        m_sut.reset();
        processEvents();
    }

protected:

    dots::type::Registry m_registry;
    std::optional<dots::Connection> m_sut;
    ::testing::MockFunction<void(dots::Connection&, const std::exception_ptr&)> m_mockTransitionHandler;
};

struct TestConnectionAsHost : TestConnectionBase
{
    TestConnectionAsHost() : TestConnectionBase(true) {}
};

struct TestConnectionAsGuest : TestConnectionBase
{
    TestConnectionAsGuest() : TestConnectionBase(false) {}
};

namespace dots
{
    inline void PrintTo(const Connection& connection, std::ostream* os)
    {
        *os << "<connection: " << connection.peerDescription() << ", 'DotsConnectionState::" << connection.state() << ">";
    }
}

namespace std
{
    inline void PrintTo(const exception_ptr& ePtr, std::ostream* os)
    {
        *os << "<std::exception_ptr: '";

        if (ePtr == nullptr)
        {
            *os << "nullptr";
        }
        else
        {
            try
            {
                std::rethrow_exception(ePtr);
            }
            catch (const std::exception& e)
            {
                *os << e.what();
            }
        }

        *os << "'>";
    }
}

#define EXPECT_TRANSITION(state_)                                                                                           \
[this]() -> auto&                                                                                                           \
{                                                                                                                           \
    return EXPECT_CALL(m_mockTransitionHandler, Call(::testing::Property(&dots::Connection::state, state_), ::testing::_)); \
}

TEST_F(TestConnectionAsHost, HandshakeWithoutAuthenticationWithoutPreloading)
{
    DOTS_EXPECT_TRANSMIT_SEQUENCE(
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::suspended);
            m_sut->asyncReceive(m_registry, nullptr, hostName(), [](dots::Connection&, dots::io::Transmission){ return true; }, m_mockTransitionHandler.AsStdFunction());
        },
        EXPECT_TRANSITION(DotsConnectionState::connecting),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::connecting);
        },
        DotsMsgHello{
            DotsMsgHello::serverName_i{ hostName() },
            DotsMsgHello::authChallenge_i{ 0u }
        },
        [this]
        {
            DOTS_SPOOF_TRANSMIT(dots::testing::TransmitSpoof{
                UninitializedId,
                DotsMsgConnect{
                    DotsMsgConnect::clientName_i{ GuestName },
                    DotsMsgConnect::preloadCache_i{ false }
                }
            });
        },
        EXPECT_TRANSITION(DotsConnectionState::connected),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::connected);
        },
        DotsMsgConnectResponse{
            DotsMsgConnectResponse::clientId_i{ m_sut->peerId() },
            DotsMsgConnectResponse::preload_i{ false },
            DotsMsgConnectResponse::accepted_i{ true }
        }
    );
    
    processEvents();
}

TEST_F(TestConnectionAsHost, HandshakeWithoutAuthenticationWithPreloading)
{
    DOTS_EXPECT_TRANSMIT_SEQUENCE(
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::suspended);
            m_sut->asyncReceive(m_registry, nullptr, hostName(), [](dots::Connection&, dots::io::Transmission){ return true; }, m_mockTransitionHandler.AsStdFunction());
        },
        EXPECT_TRANSITION(DotsConnectionState::connecting),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::connecting);
        },
        DotsMsgHello{
            DotsMsgHello::serverName_i{ hostName() },
            DotsMsgHello::authChallenge_i{ 0u }
        },
        [this]
        {
            DOTS_SPOOF_TRANSMIT(dots::testing::TransmitSpoof{
                UninitializedId,
                DotsMsgConnect{
                    DotsMsgConnect::clientName_i{ GuestName },
                    DotsMsgConnect::preloadCache_i{ true }
                }
            });
        },
        EXPECT_TRANSITION(DotsConnectionState::early_subscribe),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::early_subscribe);
        },
        DotsMsgConnectResponse{
            DotsMsgConnectResponse::clientId_i{ m_sut->peerId() },
            DotsMsgConnectResponse::preload_i{ true },
            DotsMsgConnectResponse::accepted_i{ true }
        },
        [this]
        {
            DOTS_SPOOF_TRANSMIT(dots::testing::TransmitSpoof{
                m_sut->peerId(),
                DotsMsgConnect{
                    DotsMsgConnect::preloadClientFinished_i{ true }
                }
            });
        },
        EXPECT_TRANSITION(DotsConnectionState::connected),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::connected);
        },
        DotsMsgConnectResponse{
            DotsMsgConnectResponse::preloadFinished_i{ true }
        }
    );

    processEvents();
}

TEST_F(TestConnectionAsGuest, HandshakeWithoutAuthenticationWithPreloading)
{
    DOTS_EXPECT_TRANSMIT_SEQUENCE(
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::suspended);
            m_sut->asyncReceive(m_registry, nullptr, GuestName, [](dots::Connection&, dots::io::Transmission){ return true; }, m_mockTransitionHandler.AsStdFunction());
            DOTS_SPOOF_TRANSMIT(dots::testing::TransmitSpoof{
                HostId,
                DotsMsgHello{
                    DotsMsgHello::serverName_i{ hostName() },
                    DotsMsgHello::authChallenge_i{ 0 }
                }
            });
        },
        EXPECT_TRANSITION(DotsConnectionState::connecting),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::connecting);
        },
        DotsMsgConnect{
            DotsMsgConnect::clientName_i{ GuestName },
            DotsMsgConnect::preloadCache_i{ true }
        },
        [this]
        {
            DOTS_SPOOF_TRANSMIT(dots::testing::TransmitSpoof{
                HostId,
                DotsMsgConnectResponse{
                    DotsMsgConnectResponse::clientId_i{ GuestId },
                    DotsMsgConnectResponse::preload_i{ true },
                    DotsMsgConnectResponse::accepted_i{ true }
                }
            });
        },
        EXPECT_TRANSITION(DotsConnectionState::early_subscribe),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::early_subscribe);
        },
        DotsMsgConnect{
            DotsMsgConnect::preloadClientFinished_i{ true }
        },
        [this]
        {
            DOTS_SPOOF_TRANSMIT(DotsMsgConnectResponse{
                DotsMsgConnectResponse::preloadFinished_i{ true }
            });
        },
        EXPECT_TRANSITION(DotsConnectionState::connected),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::connected);
        }
    );
    
    processEvents();
}
