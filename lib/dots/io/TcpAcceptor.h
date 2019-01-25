#pragma once
#include <boost/asio/ip/tcp.hpp>
#include <dots/eventloop/IoService.h>

namespace dots
{

typedef boost::asio::ip::tcp::acceptor TcpAcceptorBase;

class TcpAcceptor: public TcpAcceptorBase
{
public:
    TcpAcceptor();
    TcpAcceptor(IoService& ioService);

};

}