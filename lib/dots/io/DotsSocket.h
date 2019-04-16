#pragma once

#include <cstdint>
#include "dots/cpp_config.h"
#include "DotsTransportHeader.dots.h"
#include "Message.h"

class QTcpSocket;

namespace dots
{

class DotsSocket
{
public:
    typedef function<void (const Message&)> receive_callback;
    typedef function<void (int ec)> error_callback;

    DotsSocket() = default;

    virtual void start() = 0;

    virtual ~DotsSocket() = default;

    virtual void setReceiveCallback(receive_callback cb) = 0;
    virtual void setErrorCallback(error_callback cb) = 0;

    virtual int send(const DotsTransportHeader& header, const vector<uint8_t>& data = {}) = 0;

    virtual bool connect(const string &host, int port) = 0;
    virtual void disconnect() = 0;
};



typedef shared_ptr<DotsSocket> DotsSocketPtr;

}