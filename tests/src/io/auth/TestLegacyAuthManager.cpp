// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <optional>
#include <boost/algorithm/hex.hpp>
#include <dots/HostTransceiver.h>
#include <dots/testing/gtest/gtest.h>
#include <dots/type/FundamentalTypes.h>
#include <dots/io/Io.h>
#include <dots/io/auth/LegacyAuthManager.h>
#include <dots/io/auth/Digest.h>

struct TestLegacyAuthManager : ::testing::Test
{
protected:

    TestLegacyAuthManager() :
        m_transceiver{ "dots-test-host", dots::io::global_io_context() },
        m_sut{ m_transceiver }
    {
        /* do nothing */
    }

    dots::HostTransceiver m_transceiver;
    dots::io::LegacyAuthManager m_sut;
};

TEST_F(TestLegacyAuthManager, testAddUpdateRemoveRules)
{
    ASSERT_EQ(m_sut.rules().size(), 0);

    auto make_rule = [](dots::uint16_t priority, std::optional<bool> accept = std::nullopt)
    {
        DotsAuthentication rule{
            DotsAuthentication::nameSpace_i{ "" },
            DotsAuthentication::network_i{
                DotsNetwork::network_i{ "127.0.0.1" },
                DotsNetwork::prefix_i{ 8 } },
                DotsAuthentication::clientName_i{ "" },
            DotsAuthentication::priority_i{ priority }
        };

        if (accept != std::nullopt)
        {
            rule.accept = *accept;
        }

        return rule;
    };

    m_transceiver.publish(make_rule(10, true));
    m_transceiver.publish(make_rule(20, true));
    EXPECT_EQ(m_sut.rules().size(), 2);

    m_transceiver.publish(make_rule(10, false));
    EXPECT_EQ(m_sut.rules().size(), 2);

    m_transceiver.remove(make_rule(10));
    m_transceiver.remove(make_rule(20));
    EXPECT_EQ(m_sut.rules().size(), 0);
}

TEST_F(TestLegacyAuthManager, testFindNetwork)
{
    auto auth =
        m_sut.findMatchingRules(boost::asio::ip::address::from_string("127.0.0.1"), "clientName");
    EXPECT_TRUE(auth.empty());

    {
        m_transceiver.publish(DotsAuthentication{
            DotsAuthentication::nameSpace_i{ "" },
            DotsAuthentication::network_i{
                DotsNetwork::network_i{ "127.0.0.1" },
                DotsNetwork::prefix_i{ 8 }
            },
            DotsAuthentication::clientName_i{ "" },
            DotsAuthentication::priority_i{ 10 },
            DotsAuthentication::accept_i{ true }
        });
    }
    {
        m_transceiver.publish(DotsAuthentication{
            DotsAuthentication::nameSpace_i{ "" },
            DotsAuthentication::network_i{
                DotsNetwork::network_i{ "192.168.2.10" },
                DotsNetwork::prefix_i{ 24 } },
            DotsAuthentication::clientName_i{ "" },
            DotsAuthentication::priority_i{ 20 },
            DotsAuthentication::secret_i{ "mysecret" },
            DotsAuthentication::accept_i{ true }
        });
    }

    ASSERT_EQ(m_sut.rules().size(), 2);

    auth = m_sut.findMatchingRules(boost::asio::ip::address::from_string("127.0.0.1"), "clientName");
    ASSERT_EQ(auth.size(), 1);
    EXPECT_EQ(auth[0].accept, true);

    auth = m_sut.findMatchingRules(boost::asio::ip::address::from_string("192.168.1.2"), "clientName");
    ASSERT_EQ(auth.size(), 0);

    auth = m_sut.findMatchingRules(boost::asio::ip::address::from_string("192.168.2.61"), "clientName");
    ASSERT_EQ(auth.size(), 1);
    EXPECT_EQ(auth[0].accept, true);
    EXPECT_EQ(auth[0].secret, "mysecret");
}

TEST_F(TestLegacyAuthManager, testRulePriority)
{
    auto auth =
        m_sut.findMatchingRules(boost::asio::ip::address::from_string("127.0.0.1"), "clientName");
    EXPECT_TRUE(auth.empty());

    {
        m_transceiver.publish(DotsAuthentication{
            DotsAuthentication::nameSpace_i{ "" },
            DotsAuthentication::network_i{
                DotsNetwork::network_i{ "127.0.0.1" },
                DotsNetwork::prefix_i{ 8 } },
            DotsAuthentication::clientName_i{ "" },
            DotsAuthentication::priority_i{ 20 },
            DotsAuthentication::accept_i{ true }
        });
    }
    {
        m_transceiver.publish(DotsAuthentication{
            DotsAuthentication::nameSpace_i{ "" },
            DotsAuthentication::network_i{
                DotsNetwork::network_i{ "0.0.0.0" },
                DotsNetwork::prefix_i{ 0 } },
            DotsAuthentication::clientName_i{ "" },
            DotsAuthentication::priority_i{ 10 },
            DotsAuthentication::secret_i{ "remote" },
            DotsAuthentication::accept_i{ false }
        });
    }
    {
        m_transceiver.publish(DotsAuthentication{
            DotsAuthentication::nameSpace_i{ "" },
            DotsAuthentication::network_i{
                DotsNetwork::network_i{ "192.168.1.2" },
                DotsNetwork::prefix_i{ 24 } },
            DotsAuthentication::clientName_i{ "" },
            DotsAuthentication::priority_i{ 5 },
            DotsAuthentication::secret_i{ "lan" },
            DotsAuthentication::accept_i{ true }
        });
    }

    ASSERT_EQ(m_sut.rules().size(), 3);

    auth = m_sut.findMatchingRules(boost::asio::ip::address::from_string("127.0.0.1"), "clientName");
    // Entry 0.0.0.0
    ASSERT_EQ(auth.size(), 2);
    EXPECT_EQ(auth[0].accept, false);
    ASSERT_TRUE(auth[0].secret.isValid());
    EXPECT_EQ(auth[0].secret, "remote");

    // Entry 127.0.0.1
    EXPECT_EQ(auth[1].accept, true);
    ASSERT_FALSE(auth[1].secret.isValid());

    auth = m_sut.findMatchingRules(boost::asio::ip::address::from_string("192.168.1.61"), "clientName");
    ASSERT_EQ(auth.size(), 2);
    EXPECT_EQ(auth[0].accept, true);
    ASSERT_TRUE(auth[0].secret.isValid());
    EXPECT_EQ(auth[0].secret, "lan");
}

TEST_F(TestLegacyAuthManager, testDefaultPolicy)
{
    EXPECT_FALSE(m_sut.defaultPolicy().has_value());

    {
        m_transceiver.publish(DotsAuthenticationPolicy{
            DotsAuthenticationPolicy::nameSpace_i{ "" },
            DotsAuthenticationPolicy::accept_i{ false }
        });
    }

    ASSERT_TRUE(m_sut.defaultPolicy().has_value());
    EXPECT_EQ(*m_sut.defaultPolicy(), false);

    {
        m_transceiver.publish(DotsAuthenticationPolicy{
            DotsAuthenticationPolicy::nameSpace_i{ "" },
            DotsAuthenticationPolicy::accept_i{ true }
        });
    }

    ASSERT_TRUE(m_sut.defaultPolicy().has_value());
    EXPECT_EQ(*m_sut.defaultPolicy(), true);

    {
        m_transceiver.remove(DotsAuthenticationPolicy{
            DotsAuthenticationPolicy::nameSpace_i{ "" }
        });
    }

    EXPECT_FALSE(m_sut.defaultPolicy().has_value());
}

TEST_F(TestLegacyAuthManager, testCheckAuthentication)
{
    std::string secret_other = "other";
    std::string secret_lan = "lan";
    std::string secret_lan2 = "lan2";

    {
        m_transceiver.publish(DotsAuthentication{
            DotsAuthentication::nameSpace_i{ "" },
            DotsAuthentication::network_i{
                DotsNetwork::network_i{ "127.0.0.1" },
                DotsNetwork::prefix_i{ 8 } },
            DotsAuthentication::clientName_i{ "" },
            DotsAuthentication::priority_i{ 10 },
            DotsAuthentication::accept_i{ true }
        });
    }
    {
        m_transceiver.publish(DotsAuthentication{
            DotsAuthentication::nameSpace_i{ "" },
            DotsAuthentication::network_i{
                DotsNetwork::network_i{ "192.168.1.2" },
                DotsNetwork::prefix_i{ 24 } },
            DotsAuthentication::clientName_i{ "" },
            DotsAuthentication::priority_i{ 20 },
            DotsAuthentication::secret_i{ secret_lan },
            DotsAuthentication::accept_i{ true }
        });
    }
    {
        m_transceiver.publish(DotsAuthentication{
            DotsAuthentication::nameSpace_i{ "" },
            DotsAuthentication::network_i{
                DotsNetwork::network_i{ "192.168.1.2" },
                DotsNetwork::prefix_i{ 24 } },
            DotsAuthentication::clientName_i{ "" },
            DotsAuthentication::priority_i{ 21 },
            DotsAuthentication::secret_i{ secret_lan2 },
            DotsAuthentication::accept_i{ true }
        });
    }
    {
        m_transceiver.publish(DotsAuthentication{
            DotsAuthentication::nameSpace_i{ "" },
            DotsAuthentication::network_i{
                DotsNetwork::network_i{ "10.0.0.0" },
                DotsNetwork::prefix_i{ 8 } },
            DotsAuthentication::clientName_i{ "" },
            DotsAuthentication::priority_i{ 22 },
            DotsAuthentication::secret_i{ secret_other },
            DotsAuthentication::accept_i{ true }
        });
    }

    {
        m_transceiver.publish(DotsAuthenticationPolicy{
            DotsAuthenticationPolicy::nameSpace_i{ "" },
            DotsAuthenticationPolicy::accept_i{ false }
        });
    }

    ASSERT_EQ(m_sut.rules().size(), 4);

    uint64_t nonce = 12345;

    {
        auto addr = boost::asio::ip::address::from_string("127.0.0.1");
        DotsMsgConnect response{
            DotsMsgConnect::clientName_i{ "dummyClient" }
        };

        EXPECT_TRUE(m_sut.verifyResponse(addr, nonce, response));
    }

    {
        auto addr = boost::asio::ip::address::from_string("192.168.1.11");
        DotsMsgConnect response{
            DotsMsgConnect::clientName_i{ "dummyClient" },
            DotsMsgConnect::cnonce_i{ "noncense" }
        };

        auto responseHash = dots::io::Digest(nonce, *response.cnonce, *response.clientName, secret_lan).value();
        std::string responseStr = boost::algorithm::hex_lower(std::string(responseHash.begin(), responseHash.end()));

        response.authChallengeResponse.emplace(responseStr);

        EXPECT_TRUE(m_sut.verifyResponse(addr, nonce, response));
    }

    {
        auto addr = boost::asio::ip::address::from_string("192.168.1.11");
        DotsMsgConnect response{
            DotsMsgConnect::clientName_i{ "dummyClient" },
            DotsMsgConnect::cnonce_i{ "noncense" }
        };

        auto responseHash = dots::io::Digest(nonce, *response.cnonce, *response.clientName, secret_lan2).value();
        std::string responseStr = boost::algorithm::hex_lower(std::string(responseHash.begin(), responseHash.end()));

        response.authChallengeResponse.emplace(responseStr);

        EXPECT_TRUE(m_sut.verifyResponse(addr, nonce, response));
    }

    {
        auto addr = boost::asio::ip::address::from_string("10.10.1.2");
        DotsMsgConnect response{
            DotsMsgConnect::clientName_i{ "dummyClient" },
            DotsMsgConnect::cnonce_i{ "noncense" }
        };

        auto responseHash = dots::io::Digest(nonce, *response.cnonce, *response.clientName, secret_other).value();
        std::string responseStr = boost::algorithm::hex_lower(std::string(responseHash.begin(), responseHash.end()));

        response.authChallengeResponse.emplace(responseStr);

        EXPECT_TRUE(m_sut.verifyResponse(addr, nonce, response));
    }

    {
        auto addr = boost::asio::ip::address::from_string("1.2.3.4");
        DotsMsgConnect response{
            DotsMsgConnect::clientName_i{ "dummyClient" },
            DotsMsgConnect::cnonce_i{ "noncense" }
        };

        auto responseHash = dots::io::Digest(nonce, *response.cnonce, *response.clientName, "blub").value();
        std::string responseStr = boost::algorithm::hex_lower(std::string(responseHash.begin(), responseHash.end()));

        response.authChallengeResponse.emplace(responseStr);

        EXPECT_FALSE(m_sut.verifyResponse(addr, nonce, response));
    }
}

TEST_F(TestLegacyAuthManager, testCheckAuthenticationDefault)
{
    uint64_t nonce = 12345;

    {
        auto addr = boost::asio::ip::address::from_string("127.0.0.1");
        DotsMsgConnect response;
        response.clientName.emplace("dummyClient");

        EXPECT_TRUE(m_sut.verifyResponse(addr, nonce, response));
    }

    {
        auto addr = boost::asio::ip::address::from_string("192.168.1.11");
        DotsMsgConnect response{
            DotsMsgConnect::clientName_i{ "dummyClient" },
            DotsMsgConnect::cnonce_i{ "noncense" }
        };

        auto responseHash = dots::io::Digest(nonce, *response.cnonce, *response.clientName, "lan").value();
        std::string responseStr = boost::algorithm::hex_lower(std::string(responseHash.begin(), responseHash.end()));

        response.authChallengeResponse.emplace(responseStr);

        EXPECT_TRUE(m_sut.verifyResponse(addr, nonce, response));
    }
}

TEST_F(TestLegacyAuthManager, requiresAuthentication)
{
    EXPECT_FALSE(m_sut.requiresAuthentication(boost::asio::ip::address::from_string("127.0.0.1")));
    EXPECT_FALSE(m_sut.requiresAuthentication(boost::asio::ip::address::from_string("10.60.61.3")));
    EXPECT_FALSE(m_sut.requiresAuthentication(boost::asio::ip::address::from_string("10.60.61.4")));
    EXPECT_FALSE(m_sut.requiresAuthentication(boost::asio::ip::address::from_string("192.168.0.42")));

    m_transceiver.publish(DotsAuthentication{
        DotsAuthentication::nameSpace_i{ "" },
        DotsAuthentication::network_i{
            DotsNetwork::network_i{ "127.0.0.1" },
            DotsNetwork::prefix_i{ 8 }
        },
        DotsAuthentication::clientName_i{ "" },
        DotsAuthentication::priority_i{ 10 },
        DotsAuthentication::accept_i{ true }
    });

    m_transceiver.publish(DotsAuthentication{
        DotsAuthentication::nameSpace_i{ "" },
        DotsAuthentication::network_i{
            DotsNetwork::network_i{ "10.60.61.0" },
            DotsNetwork::prefix_i{ 30 }
        },
        DotsAuthentication::clientName_i{ "" },
        DotsAuthentication::priority_i{ 20 },
        DotsAuthentication::accept_i{ true }
    });

    m_transceiver.publish(DotsAuthentication{
        DotsAuthentication::nameSpace_i{ "" },
        DotsAuthentication::network_i{
            DotsNetwork::network_i{ "0.0.0.0" },
            DotsNetwork::prefix_i{ 0 }
        },
        DotsAuthentication::clientName_i{ "" },
        DotsAuthentication::priority_i{ 50 },
        DotsAuthentication::secret_i{ "foo:bar" },
        DotsAuthentication::accept_i{ true }
    });

    m_transceiver.publish(DotsAuthentication{
        DotsAuthentication::nameSpace_i{ "" },
        DotsAuthentication::network_i{
            DotsNetwork::network_i{ "0.0.0.0" },
            DotsNetwork::prefix_i{ 0 }
        },
        DotsAuthentication::clientName_i{ "" },
        DotsAuthentication::priority_i{ 51 },
        DotsAuthentication::secret_i{ "baz:qux" },
        DotsAuthentication::accept_i{ true }
    });

    m_transceiver.publish(DotsAuthenticationPolicy{
        DotsAuthenticationPolicy::nameSpace_i{ "" },
        DotsAuthenticationPolicy::accept_i{ false }
    });

    EXPECT_FALSE(m_sut.requiresAuthentication(boost::asio::ip::address::from_string("127.0.0.1")));
    EXPECT_FALSE(m_sut.requiresAuthentication(boost::asio::ip::address::from_string("10.60.61.3")));
    EXPECT_TRUE(m_sut.requiresAuthentication(boost::asio::ip::address::from_string("10.60.61.4")));
    EXPECT_TRUE(m_sut.requiresAuthentication(boost::asio::ip::address::from_string("192.168.0.42")));
}
