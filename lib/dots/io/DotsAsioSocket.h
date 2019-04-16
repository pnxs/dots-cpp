#pragma once

#include "dots/cpp_config.h"
#include "DotsSocket.h"
#include "TcpSocket.h"

namespace dots
{

class DotsAsioSocket: public DotsSocket
{
public:
    DotsAsioSocket() = default;
    DotsAsioSocket(TcpSocket socket);

    void start() override;

    void setReceiveCallback(receive_callback cb) override;
    void setErrorCallback(error_callback cb) override;

    int send(const DotsTransportHeader &header, const vector <uint8_t> &data = {}) override;

    bool connect(const string &host, int port) override;
    void disconnect() override;

private:
    void readHeaderLength();
    void readHeader();
    void readPayload();

    void handleError(const string &text, const boost::system::error_code &error);

    receive_callback m_cb;
    error_callback m_ecb;

    TcpSocket m_socket;

    uint16_t m_headerSize = 0;
    uint32_t m_payloadSize = 0;
    DotsTransportHeader m_header;
    std::vector <uint8_t> m_buffer;
    std::vector <uint8_t> m_headerBuffer;
};

}