#include "TcpAcceptor.h"
#include <dots/eventloop/AsioEventLoop.h>

namespace dots
{

TcpAcceptor::TcpAcceptor(): TcpAcceptorBase(AsioEventLoop::Instance().ioService())
{
}

TcpAcceptor::TcpAcceptor(boost::asio::io_service& ioService): TcpAcceptorBase(ioService)
{
}


}