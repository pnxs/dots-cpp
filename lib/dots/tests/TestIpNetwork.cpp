#include <gtest/gtest.h>
#include <vector>
#include <dots/tools/IpNetwork.h>

using dots::tools::IpNetwork;

struct TestIpNetwork : ::testing::Test
{
protected:

    TestIpNetwork() :
        m_parseDataValid{
            { "127.0.0.1/24", "127.0.0.1", 24, false },
            { "127.0.0.1", "127.0.0.1", 32, false },
            { "1200::ab00:1234:0:2552:7777:1313", "1200:0:ab00:1234:0:2552:7777:1313", 128, true },
            { "21da:d3:0:2f3b:2aa:ff:fe28:9c5a", "21da:d3:0:2f3b:2aa:ff:fe28:9c5a", 128, true },
            { "21da:d3:0:2f3b:2aa::fe28:9c5a/64", "21da:d3:0:2f3b:2aa:0:fe28:9c5a", 64, true }
        },
        m_parseDataInvalid{
            "300.0.0.1/24",
            "dots://127.0.0.1/24",
            "1200::ab00:1234::2552:7777:1313",
            "12xy::ab00:1234:0000:2552:7777:1313",
            "[12xy::ab00:1234:0000:2552:7777:1313]:80"
        },
        m_subnetData{
            { "192.168.1.1", "192.168.1.2/24", true, true },
            { "192.168.1.1", "192.168.1.254/24", true, true },
            { "192.168.1.1", "192.168.1.255/24", true, true },
            { "192.168.1.1", "192.168.2.2/24", false, false },
            { "192.168.1.1", "192.168.3.1/24", false, false },
            { "10.1.2.3", "10.1.2.4/16", true, true },
            { "10.1.2.3", "10.1.3.3/16", true, true },
            { "10.1.2.3", "10.1.4.4/16", true, true },
            { "10.1.2.3", "10.2.2.3/16", false, false },
            { "127.0.0.1", "127.1.2.3/8", true, true },
            { "1.2.3.4", "1.2.3.4/32", false, true },
            { "1.2.3.4", "1.2.3.5/32", false, false },
            { "1.2.3.4", "5.6.7.8/0", true, true },
            { "2001:db8::1", "2001:db8::2/64", true, true },
            { "2001:db8::1", "2001:db9::2/64", false, false },
            { "::1", "::3/8", true, true }
        }
    {
        /* do nothing */
    }

    struct ParseData
    {
        std::string networkString;
        std::string ipString;
        IpNetwork::prefix_length_t prefixLength;
        bool ipv6;
    };

    struct SubnetData
    {
        std::string lhs;
        std::string rhs;
        bool isStrictSubnetOf;
        bool isSubnetOf;
    };

    std::vector<ParseData> m_parseDataValid;
    std::vector<std::string> m_parseDataInvalid;
    std::vector<SubnetData> m_subnetData;
};

TEST_F(TestIpNetwork, ctor_ipStringWithPrefixLengthValid)
{
    for (const ParseData& parseInput : m_parseDataValid)
    {
        IpNetwork network{
            parseInput.ipString, parseInput.prefixLength
        };

        auto [address, prefixLength] = network.toPrefixAddress();

        EXPECT_EQ(address.to_string(), parseInput.ipString);
        EXPECT_EQ(prefixLength, parseInput.prefixLength);
        EXPECT_EQ(network.isIpv4(), !parseInput.ipv6);
        EXPECT_EQ(network.isIpv6(), parseInput.ipv6);
    }

    IpNetwork network{  "127.0.0.1", IpNetwork::DefaultV4PrefixLength };
    auto [address, prefixLength] = network.toPrefixAddress();

    EXPECT_EQ(address.to_string(), "127.0.0.1");
    EXPECT_EQ(prefixLength, IpNetwork::DefaultV4PrefixLength);
}

TEST_F(TestIpNetwork, ctor_ipStringWithPrefixLengthInvalid)
{
    for (const std::string& networkString : m_parseDataInvalid)
    {
        EXPECT_THROW(IpNetwork(networkString, 32), std::runtime_error);
    }
}

TEST_F(TestIpNetwork, ctor_networkStringValid)
{
    for (const ParseData& parseInput : m_parseDataValid)
    {
        IpNetwork network{ parseInput.networkString };
        auto [address, prefixLength] = network.toPrefixAddress();

        EXPECT_EQ(address.to_string(), parseInput.ipString);
        EXPECT_EQ(prefixLength, parseInput.prefixLength);
        EXPECT_EQ(network.isIpv4(), !parseInput.ipv6);
        EXPECT_EQ(network.isIpv6(), parseInput.ipv6);
    }
}

TEST_F(TestIpNetwork, ctor_networkStringInvalid)
{
    for (const std::string& networkString : m_parseDataInvalid)
    {
        EXPECT_THROW(IpNetwork{ networkString }, std::runtime_error);
    }
}

TEST_F(TestIpNetwork, ctor_ipAddress)
{
    for (const ParseData& parseInput : m_parseDataValid)
    {
        IpNetwork network{ boost::asio::ip::address::from_string(parseInput.ipString) };
        auto [address, prefixLength] = network.toPrefixAddress();

        EXPECT_EQ(address.to_string(), parseInput.ipString);
        EXPECT_EQ(prefixLength, parseInput.ipv6 ? IpNetwork::DefaultV6PrefixLength : IpNetwork::DefaultV4PrefixLength);
        EXPECT_EQ(network.isIpv4(), !parseInput.ipv6);
        EXPECT_EQ(network.isIpv6(), parseInput.ipv6);
    }
}

TEST_F(TestIpNetwork, toString)
{
    for (const ParseData& parseInput : m_parseDataValid)
    {
        IpNetwork network{ parseInput.networkString };
        std::string networkString = network.toString();

        EXPECT_EQ(networkString, parseInput.ipString + "/" + std::to_string(parseInput.prefixLength));
    }
}

TEST_F(TestIpNetwork, isSubnetOf)
{
    for (const SubnetData& subnetData : m_subnetData)
    {
        IpNetwork lhs{ subnetData.lhs };
        IpNetwork rhs{ subnetData.rhs };

        EXPECT_EQ(lhs.isStrictSubnetOf(rhs), subnetData.isStrictSubnetOf);
        EXPECT_EQ(lhs.isSubnetOf(rhs), subnetData.isSubnetOf);
    }
}