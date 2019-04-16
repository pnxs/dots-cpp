#pragma once

#include <dots/eventloop/AsioEventLoop.h>

#include <boost/asio.hpp>

namespace dots {

class TcpSocket: public boost::asio::ip::tcp::socket
{

    typedef boost::asio::ip::tcp::socket Base;

public:

    TcpSocket();
    TcpSocket(boost::asio::io_service& ioService);
    TcpSocket(TcpSocket&& socket);
    ~TcpSocket();

    template<typename MutableBufferSequence, typename ReadHandler>
    void asyncRead(const MutableBufferSequence& buffers, BOOST_ASIO_MOVE_ARG(ReadHandler) handler)
    {
        boost::asio::async_read(*this, buffers, handler);
    }

    ssize_t receive(void *buffer, size_t length);
    ssize_t send(const void *buffer, size_t length);
    ssize_t send(const boost::asio::const_buffer *a, size_t length);
    int nativeHandle();
    void nonBlocking(bool mode);
    void close();

    int sendBufferSize() const;
    void setSendBufferSize(int value);

	mutable boost::system::error_code ec;
};


}