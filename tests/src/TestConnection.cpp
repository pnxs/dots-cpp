// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <sstream>
#include <optional>
#include <dots/Connection.h>
#include <dots/io/Io.h>
#include <dots/type/Registry.h>
#include <dots/testing/gtest/expectations/TransmitExpectation.h>
#include <dots/testing/gtest/ChannelTestBase.h>
#include <DotsTestStruct.dots.h>

#define EXPECT_TRANSITION                                                                                                  \
[this](DotsConnectionState state) -> auto&                                                                                 \
{                                                                                                                          \
    return EXPECT_CALL(m_mockTransitionHandler, Call(::testing::Property(&dots::Connection::state, state), ::testing::_)); \
}

struct TestConnectionBase : dots::testing::ChannelTestBase
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
                .errorCode = 0
            });
        }

        EXPECT_TRANSITION(DotsConnectionState::closed).Times(::testing::AnyNumber());

        m_sut.reset();
        processEvents();
    }

protected:

    dots::type::Registry m_registry;
    std::optional<dots::Connection> m_sut;
    ::testing::MockFunction<bool(dots::Connection&, dots::io::Transmission)> m_mockReceiveHandler;
    ::testing::MockFunction<void(dots::Connection&, std::exception_ptr)> m_mockTransitionHandler;
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
            .serverName = hostName(),
            .authChallenge = 0u
        }),
        [this]
        {
            SPOOF_DOTS_TRANSMIT_FROM_SENDER(
                UninitializedId,
                DotsMsgConnect{
                    .clientName = GuestName,
                    .preloadCache = false
                }
            );
        },
        EXPECT_TRANSITION(DotsConnectionState::connected),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::connected);
        },
        EXPECT_DOTS_TRANSMIT(DotsMsgConnectResponse{
            .clientId = m_sut->peerId(),
            .accepted = true,
            .preload = false
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
            .serverName = hostName(),
            .authChallenge = 0u
        }),
        [this]
        {
            SPOOF_DOTS_TRANSMIT_FROM_SENDER(
                UninitializedId,
                DotsMsgConnect{
                    .clientName = GuestName,
                    .preloadCache = true
                }
            );
        },
        EXPECT_TRANSITION(DotsConnectionState::early_subscribe),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::early_subscribe);
        },
        EXPECT_DOTS_TRANSMIT(DotsMsgConnectResponse{
            .clientId = m_sut->peerId(),
            .accepted = true,
            .preload = true
        }),
        [this]
        {
            SPOOF_DOTS_TRANSMIT_FROM_SENDER(
                m_sut->peerId(),
                DotsMsgConnect{
                    .preloadClientFinished = true
                }
            );
        },
        EXPECT_TRANSITION(DotsConnectionState::connected),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::connected);
        },
        EXPECT_DOTS_TRANSMIT(DotsMsgConnectResponse{
            .preloadFinished = true
        })
    );

    processEvents();
}

TEST_F(TestConnectionAsHost, RejectInstancesThatAreMissingKeyProperties)
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
            .serverName = hostName(),
            .authChallenge = 0u
        }),
        [this]
        {
            SPOOF_DOTS_TRANSMIT_FROM_SENDER(
                UninitializedId,
                DotsMsgConnect{
                    .clientName = GuestName,
                    .preloadCache = false
                }
            );
        },
        EXPECT_TRANSITION(DotsConnectionState::connected),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::connected);
        },
        EXPECT_DOTS_TRANSMIT(DotsMsgConnectResponse{
            .clientId = m_sut->peerId(),
            .accepted = true,
            .preload = false
        }),
        [this]
        {
            SPOOF_DOTS_TRANSMIT_FROM_SENDER(
                m_sut->peerId(),
                DotsTestStruct{
                    .stringField = "foobar"
                }
            );
        },
        EXPECT_TRANSITION(DotsConnectionState::closed),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::closed);
        },
        EXPECT_DOTS_TRANSMIT(DotsMsgError{
            .errorCode = 1
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
                    .serverName = hostName(),
                    .authChallenge = 0
                }
            );
        },
        EXPECT_TRANSITION(DotsConnectionState::connecting),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::connecting);
        },
        EXPECT_DOTS_TRANSMIT(DotsMsgConnect{
            .clientName = GuestName,
            .preloadCache = true
        }),
        [this]
        {
            SPOOF_DOTS_TRANSMIT_FROM_SENDER(
                HostId,
                DotsMsgConnectResponse{
                    .clientId = GuestId,
                    .accepted = true,
                    .preload = true
                }
            );
        },
        EXPECT_TRANSITION(DotsConnectionState::early_subscribe),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::early_subscribe);
        },
        EXPECT_DOTS_TRANSMIT(DotsMsgConnect{
            .preloadClientFinished = true
        }),
        [this]
        {
            SPOOF_DOTS_TRANSMIT_FROM_SENDER(
                HostId,
                DotsMsgConnectResponse{
                    .preloadFinished = true
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

TEST_F(TestConnectionAsGuest, RejectInstancesThatAreMissingKeyProperties)
{
    DOTS_EXPECTATION_SEQUENCE(
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::suspended);
            m_sut->asyncReceive(m_registry, nullptr, GuestName, m_mockReceiveHandler.AsStdFunction(), m_mockTransitionHandler.AsStdFunction());
            SPOOF_DOTS_TRANSMIT_FROM_SENDER(
                HostId,
                DotsMsgHello{
                    .serverName = hostName(),
                    .authChallenge = 0
                }
            );
        },
        EXPECT_TRANSITION(DotsConnectionState::connecting),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::connecting);
        },
        EXPECT_DOTS_TRANSMIT(DotsMsgConnect{
            .clientName = GuestName,
            .preloadCache = true
        }),
        [this]
        {
            SPOOF_DOTS_TRANSMIT_FROM_SENDER(
                HostId,
                DotsMsgConnectResponse{
                    .clientId = GuestId,
                    .accepted = true,
                    .preload = true
                }
            );
        },
        EXPECT_TRANSITION(DotsConnectionState::early_subscribe),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::early_subscribe);
        },
        EXPECT_DOTS_TRANSMIT(DotsMsgConnect{
            .preloadClientFinished = true
        }),
        [this]
        {
            SPOOF_DOTS_TRANSMIT_FROM_SENDER(
                HostId,
                DotsMsgConnectResponse{
                    .preloadFinished = true
                }
            );
        },
        EXPECT_TRANSITION(DotsConnectionState::connected),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::connected);
            SPOOF_DOTS_TRANSMIT_FROM_SENDER(
                HostId,
                DotsTestStruct{
                    .stringField = "foobar"
                }
            );
        },
        EXPECT_TRANSITION(DotsConnectionState::closed),
        [this]
        {
            EXPECT_EQ(m_sut->state(), DotsConnectionState::closed);
        },
        EXPECT_DOTS_TRANSMIT(DotsMsgError{
            .errorCode = 1
        })
    );
    
    processEvents();
}
