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
    void close();
    void nonBlocking(bool mode);

	mutable boost::system::error_code ec;
};


}