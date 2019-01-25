#pragma once
#include "dots/cpp_config.h"
#include <boost/asio/ip/tcp.hpp>

namespace dots {

typedef boost::asio::ip::address IpAddr;
typedef boost::asio::ip::address_v4 IpAddrV4;
typedef boost::asio::ip::address_v6 IpAddrV6;

class TcpEndpoint: public boost::asio::ip::tcp::endpoint
{
    typedef boost::asio::ip::tcp::endpoint Base;
public:

    TcpEndpoint(const IpAddr &addr, uint16_t port) : Base(addr, port) {}
    TcpEndpoint(const string &addr, uint16_t port) : Base(IpAddr::from_string(addr), port) {}
    TcpEndpoint(uint32_t addr, uint16_t port) : Base(IpAddrV4(addr), port) {}
    TcpEndpoint(uint16_t port) : Base(IpAddrV4::any(), port) {}

    string toString() const;

};

}