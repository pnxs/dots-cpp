#pragma once
#include <string>
#include <variant>
#include <boost/asio.hpp>

namespace dots::tools
{
    struct IpNetwork
    {
        using address_t = boost::asio::ip::address;
        using prefix_length_t = unsigned short;
        using network_v4_t = boost::asio::ip::network_v4;
        using network_v6_t = boost::asio::ip::network_v6;
        using network_t = std::variant<network_v4_t, network_v6_t>;

        static constexpr prefix_length_t DefaultV4PrefixLength = 32;
        static constexpr prefix_length_t DefaultV6PrefixLength = 128;

        IpNetwork(network_t network);
        IpNetwork(const std::string& address, prefix_length_t prefixLength);
        IpNetwork(const std::string& network);
        IpNetwork(const boost::asio::ip::address& address);
        IpNetwork(const IpNetwork& other) = default;
        IpNetwork(IpNetwork&& other) = default;
        ~IpNetwork() = default;

        IpNetwork& operator = (const IpNetwork& rhs) = default;
        IpNetwork& operator = (IpNetwork&& rhs) = default;

        bool isIpv4() const;
        bool isIpv6() const;

        bool isStrictSubnetOf(const IpNetwork& network);
        bool isSubnetOf(const IpNetwork& network);

        auto toPrefixAddress() -> std::pair<address_t, prefix_length_t>;
        std::string toString() const;

    private:

        network_t m_network;
    };
}
