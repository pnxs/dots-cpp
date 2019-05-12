#pragma once
#include <string_view>
#include <asio.hpp>
#include <dots/io/services/Channel.h>

namespace dots
{
	struct TcpChannel : Channel
	{
		TcpChannel(asio::io_context& ioContext, const std::string_view& host, const std::string_view& port);
		TcpChannel(asio::ip::tcp::socket&& socket);
		TcpChannel(const TcpChannel& other) = delete;
		TcpChannel(TcpChannel&& other) = delete;
		virtual ~TcpChannel() = default;

		TcpChannel& operator = (const TcpChannel& rhs) = delete;
		TcpChannel& operator = (TcpChannel&& rhs) = delete;

		void asyncReceive(receive_handler_t&& receiveHandler, error_handler_t&& errorHandler) override;
		void transmit(const DotsTransportHeader& header, const type::Struct& instance) override;

	private:

		void asynReadHeaderLength();
		void asyncReadHeader();
		void asyncReadPayload();

		void handleError(const std::string& text, const asio::error_code& error);

		receive_handler_t m_cb;
		error_handler_t m_ecb;

		asio::ip::tcp::socket m_socket;

		uint16_t m_headerSize = 0;
		uint32_t m_payloadSize = 0;
		DotsTransportHeader m_header;
		std::vector<uint8_t> m_buffer;
		std::vector<uint8_t> m_headerBuffer;
	};
}