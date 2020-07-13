#include <dots/tools/IpNetwork.h>

namespace dots::tools
{
    IpNetwork::IpNetwork(network_t network) :
        m_network{ std::move(network) }
    {
        /* do nothing */
    }

    IpNetwork::IpNetwork(const std::string& address, prefix_length_t prefixLength) :
        IpNetwork([&]() -> network_t
        {
            if (auto address_ = boost::asio::ip::make_address(address); address_.is_v4())
            {
                return boost::asio::ip::make_network_v4(address_.to_v4(), prefixLength);
            }
            else
            {
                return boost::asio::ip::make_network_v6(address_.to_v6(), prefixLength);
            }
        }())
    {
        /* do nothing */
    }

    IpNetwork::IpNetwork(const std::string& network) :
        IpNetwork([&]() -> network_t
        {
            boost::system::error_code ec;

            if (std::string::size_type pos = network.find_last_of("/"); pos == std::string::npos)
            {
                if (auto address = boost::asio::ip::make_address(network); address.is_v4())
                {
                    return boost::asio::ip::make_network_v4(address.to_v4(), DefaultV4PrefixLength);
                }
                else
                {
                    return boost::asio::ip::make_network_v6(address.to_v6(), DefaultV6PrefixLength);
                }
            }
            else if (auto networkV4 = boost::asio::ip::make_network_v4(network, ec); !ec.failed())
            {
                return networkV4;
            }
            else if (auto networkV6 = boost::asio::ip::make_network_v6(network, ec); !ec.failed())
            {
                return networkV6;
            }
            else
            {
                throw std::runtime_error("could not create network from invalid input string: '" + network + "'");
            }
        }())
    {
        /* do nothing */
    }

    IpNetwork::IpNetwork(const boost::asio::ip::address& address) :
        IpNetwork([&]() -> network_t
        {
            if (address.is_v4())
            {
                return boost::asio::ip::make_network_v4(address.to_v4(), DefaultV4PrefixLength);
            }
            else
            {
                return boost::asio::ip::make_network_v6(address.to_v6(), DefaultV6PrefixLength);
            }
        }())
    {
        /* do nothing */
    }

    bool IpNetwork::isIpv4() const
    {
        return std::holds_alternative<network_v4_t>(m_network);
    }

    bool IpNetwork::isIpv6() const
    {
        return std::holds_alternative<network_v6_t>(m_network);
    }

    bool IpNetwork::isStrictSubnetOf(const IpNetwork& network)
    {
        if (m_network.index() != network.m_network.index())
        {
            return false;
        }
        else if (isIpv4())
        {
            return std::get<network_v4_t>(m_network).is_subnet_of(std::get<network_v4_t>(network.m_network));
        }
        else/* if (isIpv6())*/
        {
            return std::get<network_v6_t>(m_network).is_subnet_of(std::get<network_v6_t>(network.m_network));
        }
    }

    bool IpNetwork::isSubnetOf(const IpNetwork& network)
    {
        if (m_network.index() != network.m_network.index())
        {
            return false;
        }
        else
        {
            return isStrictSubnetOf(network) || m_network == network.m_network;
        }
    }

    auto IpNetwork::toPrefixAddress() -> std::pair<address_t, prefix_length_t>
    {
        return std::visit([](const auto& network){ return std::make_pair(address_t{ network.address() }, network.prefix_length()); }, m_network);
    }

    std::string IpNetwork::toString() const
    {
        return std::visit([](const auto& network){ return network.to_string(); }, m_network);
    }
}