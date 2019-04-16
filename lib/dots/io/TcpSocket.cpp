#include "TcpSocket.h"

#include <dots/eventloop/AsioEventLoop.h>
#include "ConstBufSeq.h"
#include <boost/asio.hpp>

namespace dots {


TcpSocket::TcpSocket(): Base(AsioEventLoop::Instance().ioService())
{}

TcpSocket::TcpSocket(boost::asio::io_service&ioService): Base(ioService)
{}

TcpSocket::TcpSocket(TcpSocket&& socket): Base(std::move(socket))
{}

TcpSocket::~TcpSocket()
{}

void TcpSocket::nonBlocking(bool mode)
{
    Base::non_blocking(mode);
}

void TcpSocket::close()
{
    Base::close();
}


}