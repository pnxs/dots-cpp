#pragma once
#include <asio.hpp>
#include <dots/io/services/Channel.h>

namespace dots
{
	struct TcpChannel : Channel
	{
		TcpChannel(asio::io_context& ioContext, const std::string& host, int port);
		TcpChannel(asio::ip::tcp::socket&& socket);
		TcpChannel(const TcpChannel& other) = delete;
		TcpChannel(TcpChannel&& other) = delete;
		virtual ~TcpChannel() = default;

		TcpChannel& operator = (const TcpChannel& rhs) = delete;
		TcpChannel& operator = (TcpChannel&& rhs) = delete;

		void asyncReceive(std::function<void(const Message&)>&& receiveHandler, std::function<void(int ec)>&& errorHandler) override;
		int transmit(const DotsTransportHeader& header, const std::vector <uint8_t>& data = {}) override;

	private:

		bool connect(const std::string& host, int port);
		void readHeaderLength();
		void readHeader();
		void readPayload();

		template<typename MutableBufferSequence, typename ReadHandler>
		void asyncRead(const MutableBufferSequence& buffers, ASIO_MOVE_ARG(ReadHandler) handler)
		{
			asio::async_read(m_socket, buffers, handler);
		}

		void handleError(const std::string& text, const asio::error_code& error);

		std::function<void(const Message&)> m_cb;
		std::function<void(int ec)> m_ecb;

		asio::ip::tcp::socket m_socket;

		uint16_t m_headerSize = 0;
		uint32_t m_payloadSize = 0;
		DotsTransportHeader m_header;
		std::vector <uint8_t> m_buffer;
		std::vector <uint8_t> m_headerBuffer;
	};
}