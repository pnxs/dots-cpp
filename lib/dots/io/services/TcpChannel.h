#pragma once
#include <string_view>
#include <boost/asio.hpp>
#include <dots/io/services/Channel.h>

namespace dots
{
	struct TcpChannel : Channel
	{
		TcpChannel(Channel::key_t key, boost::asio::io_context& ioContext, const std::string_view& host, const std::string_view& port);
		TcpChannel(Channel::key_t key, boost::asio::ip::tcp::socket&& socket);
		TcpChannel(const TcpChannel& other) = delete;
		TcpChannel(TcpChannel&& other) = delete;
		virtual ~TcpChannel() = default;

		TcpChannel& operator = (const TcpChannel& rhs) = delete;
		TcpChannel& operator = (TcpChannel&& rhs) = delete;

	protected:

		void asyncReceiveImpl() override;
		void transmitImpl(const DotsTransportHeader& header, const type::Struct& instance) override;

	private:

		void asynReadHeaderLength();
		void asyncReadHeader();
		void asyncReadInstance();

		void verifyErrorCode(const boost::system::error_code& error);

		receive_handler_t m_cb;
		error_handler_t m_ecb;

		boost::asio::ip::tcp::socket m_socket;
		uint16_t m_headerSize;
		DotsTransportHeader m_header;
		std::vector<uint8_t> m_headerBuffer;
		std::vector<uint8_t> m_instanceBuffer;
	};
}