#pragma once
#include <string_view>
#include <boost/asio.hpp>
#include <dots/io/services/Channel.h>

namespace dots::io::posix
{
	struct UdsChannel : Channel
	{
		UdsChannel(boost::asio::io_context& ioContext, const std::string_view& path);
		UdsChannel(boost::asio::local::stream_protocol::socket&& socket);
		UdsChannel(const UdsChannel& other) = delete;
		UdsChannel(UdsChannel&& other) = delete;
		virtual ~UdsChannel() noexcept = default;

		UdsChannel& operator = (const UdsChannel& rhs) = delete;
		UdsChannel& operator = (UdsChannel&& rhs) = delete;

	protected:

		void asyncReceiveImpl() override;
		void transmitImpl(const DotsTransportHeader& header, const type::Struct& instance) override;

	private:

		void asynReadHeaderLength();
		void asyncReadHeader();
		void asyncReadInstance();

		void verifyErrorCode(const boost::system::error_code& error);

		static void IgnorePipeSignals();

		receive_handler_t m_cb;
		error_handler_t m_ecb;

		boost::asio::local::stream_protocol::endpoint m_endpoint;
		boost::asio::local::stream_protocol::socket m_socket;
		uint16_t m_headerSize;
		DotsTransportHeader m_header;
		std::vector<uint8_t> m_headerBuffer;
		std::vector<uint8_t> m_instanceBuffer;
	};
}