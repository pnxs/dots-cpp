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
            EXPECT_DOTS_TRANSMIT(DotsMsgError{
                DotsMsgError::errorCode_i{ 0 }
            });
        }

        m_sut.reset();
        processEvents();
    }

protected:

    dots::type::Registry m_registry;
    std::optional<dots::Connection> m_sut;
    ::testing::MockFunction<bool(dots::Connection&, dots::io::Transmission)> m_mockReceiveHandler;
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

#define EXPECT_TRANSITION                                                                                                  \
[this](DotsConnectionState state) -> auto&                                                                                 \
{                                                                                                                          \
    return EXPECT_CALL(m_mockTransitionHandler, Call(::testing::Property(&dots::Connection::state, state), ::testing::_)); \
}

TEST_F(TestConnectionAsHost, HandshakeWithoutAuthenticationWithoutPreloading)
{
    DOTS_EXPECTATION_SEQUENCE(
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::suspended);
            m_sut->asyncReceive(m_registry, nullptr, hostName(), m_mockReceiveHandler.AsStdFunction(), m_mockTransitionHandler.AsStdFunction());
        },
        EXPECT_TRANSITION(DotsConnectionState::connecting),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::connecting);
        },
        EXPECT_DOTS_TRANSMIT(DotsMsgHello{
            DotsMsgHello::serverName_i{ hostName() },
            DotsMsgHello::authChallenge_i{ 0u }
        }),
        [this]
        {
            SPOOF_DOTS_TRANSMIT_FROM_SENDER(
                UninitializedId,
                DotsMsgConnect{
                    DotsMsgConnect::clientName_i{ GuestName },
                    DotsMsgConnect::preloadCache_i{ false }
                }
            );
        },
        EXPECT_TRANSITION(DotsConnectionState::connected),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::connected);
        },
        EXPECT_DOTS_TRANSMIT(DotsMsgConnectResponse{
            DotsMsgConnectResponse::clientId_i{ m_sut->peerId() },
            DotsMsgConnectResponse::preload_i{ false },
            DotsMsgConnectResponse::accepted_i{ true }
        })
    );
    
    processEvents();
}

TEST_F(TestConnectionAsHost, HandshakeWithoutAuthenticationWithPreloading)
{
    DOTS_EXPECTATION_SEQUENCE(
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::suspended);
            m_sut->asyncReceive(m_registry, nullptr, hostName(), m_mockReceiveHandler.AsStdFunction(), m_mockTransitionHandler.AsStdFunction());
        },
        EXPECT_TRANSITION(DotsConnectionState::connecting),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::connecting);
        },
        EXPECT_DOTS_TRANSMIT(DotsMsgHello{
            DotsMsgHello::serverName_i{ hostName() },
            DotsMsgHello::authChallenge_i{ 0u }
        }),
        [this]
        {
            SPOOF_DOTS_TRANSMIT_FROM_SENDER(
                UninitializedId,
                DotsMsgConnect{
                    DotsMsgConnect::clientName_i{ GuestName },
                    DotsMsgConnect::preloadCache_i{ true }
                }
            );
        },
        EXPECT_TRANSITION(DotsConnectionState::early_subscribe),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::early_subscribe);
        },
        EXPECT_DOTS_TRANSMIT(DotsMsgConnectResponse{
            DotsMsgConnectResponse::clientId_i{ m_sut->peerId() },
            DotsMsgConnectResponse::preload_i{ true },
            DotsMsgConnectResponse::accepted_i{ true }
        }),
        [this]
        {
            SPOOF_DOTS_TRANSMIT_FROM_SENDER(
                m_sut->peerId(),
                DotsMsgConnect{
                    DotsMsgConnect::preloadClientFinished_i{ true }
                }
            );
        },
        EXPECT_TRANSITION(DotsConnectionState::connected),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::connected);
        },
        EXPECT_DOTS_TRANSMIT(DotsMsgConnectResponse{
            DotsMsgConnectResponse::preloadFinished_i{ true }
        })
    );

    processEvents();
}

TEST_F(TestConnectionAsGuest, HandshakeWithoutAuthenticationWithPreloading)
{
    DOTS_EXPECTATION_SEQUENCE(
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::suspended);
            m_sut->asyncReceive(m_registry, nullptr, GuestName, m_mockReceiveHandler.AsStdFunction(), m_mockTransitionHandler.AsStdFunction());
            SPOOF_DOTS_TRANSMIT_FROM_SENDER(
                HostId,
                DotsMsgHello{
                    DotsMsgHello::serverName_i{ hostName() },
                    DotsMsgHello::authChallenge_i{ 0 }
                }
            );
        },
        EXPECT_TRANSITION(DotsConnectionState::connecting),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::connecting);
        },
        EXPECT_DOTS_TRANSMIT(DotsMsgConnect{
            DotsMsgConnect::clientName_i{ GuestName },
            DotsMsgConnect::preloadCache_i{ true }
        }),
        [this]
        {
            SPOOF_DOTS_TRANSMIT_FROM_SENDER(
                HostId,
                DotsMsgConnectResponse{
                    DotsMsgConnectResponse::clientId_i{ GuestId },
                    DotsMsgConnectResponse::preload_i{ true },
                    DotsMsgConnectResponse::accepted_i{ true }
                }
            );
        },
        EXPECT_TRANSITION(DotsConnectionState::early_subscribe),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::early_subscribe);
        },
        EXPECT_DOTS_TRANSMIT(DotsMsgConnect{
            DotsMsgConnect::preloadClientFinished_i{ true }
        }),
        [this]
        {
            SPOOF_DOTS_TRANSMIT_FROM_SENDER(
                HostId,
                DotsMsgConnectResponse{
                    DotsMsgConnectResponse::preloadFinished_i{ true }
                }
            );
        },
        EXPECT_TRANSITION(DotsConnectionState::connected),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::connected);
        }
    );
    
    processEvents();
}
