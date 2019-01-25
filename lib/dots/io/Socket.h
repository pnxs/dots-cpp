#pragma once

#include "boost/asio/buffer.hpp"
#include "boost/asio/ip/address.hpp"
#include "boost/asio/ip/udp.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/local/stream_protocol.hpp"

#define USE_LOCAL_SOCKETS BOOST_ASIO_HAS_LOCAL_SOCKETS

namespace dots {

class Socket
{
public:
    mutable boost::system::error_code ec;

    virtual ~Socket() {}

    virtual ssize_t receive(void *buffer, size_t length) = 0;
    virtual ssize_t send(const void *buffer, size_t length) = 0;
    virtual ssize_t send(const boost::asio::const_buffer *a, size_t length) = 0;
    virtual int nativeHandle() = 0;
    virtual void nonBlocking(bool mode) = 0;
    virtual void close() = 0;

    virtual int sendBufferSize() const = 0;
    virtual void setSendBufferSize(int value) = 0;

};

}