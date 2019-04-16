#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <dots/eventloop/AsioEventLoop.h>

namespace dots
{

class TcpResolver: public boost::asio::ip::tcp::resolver
{
    typedef boost::asio::ip::tcp::resolver base;
public:
    TcpResolver();
    TcpResolver(boost::asio::io_service&ioService);
    ~TcpResolver();

};

}