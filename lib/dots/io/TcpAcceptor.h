#pragma once
#include <boost/asio/ip/tcp.hpp>
#include <dots/eventloop/AsioEventLoop.h>

namespace dots
{

typedef boost::asio::ip::tcp::acceptor TcpAcceptorBase;

class TcpAcceptor: public TcpAcceptorBase
{
public:
    TcpAcceptor();
    TcpAcceptor(boost::asio::io_service& ioService);

};

}