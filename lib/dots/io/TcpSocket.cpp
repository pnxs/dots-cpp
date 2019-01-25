#include "TcpSocket.h"

#include <dots/eventloop/IoService.h>
#include "ConstBufSeq.h"
#include <boost/asio.hpp>

namespace dots {


TcpSocket::TcpSocket(): Base(ioService())
{}

TcpSocket::TcpSocket(IoService &ioService): Base(ioService)
{}

TcpSocket::TcpSocket(TcpSocket&& socket): Base(std::move(socket))
{}

TcpSocket::~TcpSocket()
{}


ssize_t TcpSocket::receive(void *buffer, size_t length)
{
    ssize_t received = boost::asio::ip::tcp::socket::receive(boost::asio::buffer(buffer, length), 0, ec);

    if (ec)
    {
        return (ec == boost::asio::error::eof) ? 0 : -1;
    }

    return received;
}

ssize_t TcpSocket::send(const void *buffer, size_t length)
{
    ssize_t ret = boost::asio::ip::tcp::socket::send(boost::asio::buffer(buffer, length), 0, ec);
    return ec ? -1 : ret;
}

ssize_t TcpSocket::send(const boost::asio::const_buffer *a, size_t length)
{
    ssize_t ret = boost::asio::ip::tcp::socket::send(ConstBufSeq{a, length}, 0, ec);
    return ec ? -1 : ret;
}

int TcpSocket::nativeHandle()
{
    return Base::native_handle();
}

void TcpSocket::nonBlocking(bool mode)
{
    Base::non_blocking(mode);
}

void TcpSocket::close()
{
    Base::close();
}

int TcpSocket::sendBufferSize() const
{
    send_buffer_size option;
    get_option(option);
    return option.value();
}

void TcpSocket::setSendBufferSize(int value)
{
   set_option(send_buffer_size(value));
}


}