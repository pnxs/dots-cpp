#pragma once
#include <asio.hpp>
#include "dots/cpp_config.h"
#include "DotsSocket.h"

namespace dots
{

class DotsAsioSocket: public DotsSocket
{
public:
    DotsAsioSocket();
    DotsAsioSocket(asio::ip::tcp::socket socket);

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

	template<typename MutableBufferSequence, typename ReadHandler>
	void asyncRead(const MutableBufferSequence& buffers, ASIO_MOVE_ARG(ReadHandler) handler)
	{
		asio::async_read(m_socket, buffers, handler);
	}

    void handleError(const string &text, const asio::error_code& error);

    receive_callback m_cb;
    error_callback m_ecb;

	asio::ip::tcp::socket m_socket;

    uint16_t m_headerSize = 0;
    uint32_t m_payloadSize = 0;
    DotsTransportHeader m_header;
    std::vector <uint8_t> m_buffer;
    std::vector <uint8_t> m_headerBuffer;
};

}