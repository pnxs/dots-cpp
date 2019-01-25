#pragma once

#include "dots/cpp_config.h"
#include "TcpSocket.h"
#include "DotsTransportHeader.dots.h"

namespace dots
{

typedef function<void (const DotsTransportHeader&, const vector<uint8_t>&)> ReceiverCallback;

class Receiver
{
public:
    Receiver(ReceiverCallback cb);

    void start(shared_ptr<TcpSocket> socket);

private:
    void readHeaderLength();
    void readHeader();
    void readPayload();

    ReceiverCallback m_cb;

    shared_ptr<TcpSocket> m_socket;

    uint16_t m_headerSize = 0;
    uint32_t m_payloadSize = 0;
    DotsTransportHeader m_header;
    std::vector<uint8_t > m_buffer;
};

}