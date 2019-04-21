#pragma once

#include <cstdint>
#include <dots/cpp_config.h>
#include <DotsTransportHeader.dots.h>
#include <dots/io/Message.h>

class QTcpSocket;

namespace dots
{

class Channel
{
public:
    typedef function<void (const Message&)> receive_callback;
    typedef function<void (int ec)> error_callback;

    Channel() = default;

    virtual void start() = 0;

    virtual ~Channel() = default;

    virtual void setReceiveCallback(receive_callback cb) = 0;
    virtual void setErrorCallback(error_callback cb) = 0;

    virtual int send(const DotsTransportHeader& header, const vector<uint8_t>& data = {}) = 0;

    virtual bool connect(const string &host, int port) = 0;
    virtual void disconnect() = 0;
};



typedef shared_ptr<Channel> ChannelPtr;

}