#pragma once
#include <string_view>
#include <optional>
#include <boost/asio.hpp>
#include <dots/io/Channel.h>
#include <DotsTransportHeader.dots.h>

namespace dots::io
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

		const Medium& medium() const override;

	protected:

		void asyncReceiveImpl() override;
		void transmitImpl(const DotsHeader& header, const type::Struct& instance) override;

	private:

		static constexpr char TcpSocketCategory[] = "tcp";

		void asynReadHeaderLength();
		void asyncReadHeader();
		void asyncReadInstance();

		void verifyErrorCode(const boost::system::error_code& error);

		receive_handler_t m_cb;
		error_handler_t m_ecb;

		boost::asio::ip::tcp::socket m_socket;
		uint16_t m_headerSize;
		DotsTransportHeader m_transportHeader;
		std::vector<uint8_t> m_headerBuffer;
		std::vector<uint8_t> m_instanceBuffer;
		std::optional<Medium> m_medium;
	};
}