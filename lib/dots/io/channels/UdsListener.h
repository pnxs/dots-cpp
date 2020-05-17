#pragma once
#include <string_view>
#include <optional>
#include <boost/asio.hpp>
#include <dots/io/Listener.h>
#include <dots/io/channels/UdsChannel.h>

namespace dots::io::posix
{
	struct UdsListener : Listener
	{
		UdsListener(boost::asio::io_context& ioContext, const std::string_view& path, std::optional<int> backlog = std::nullopt);
		UdsListener(const UdsListener& other) = delete;
		UdsListener(UdsListener&& other) = delete;
		~UdsListener();

		UdsListener& operator = (const UdsListener& rhs) = delete;
		UdsListener& operator = (UdsListener&& rhs) = delete;

	protected:

		void asyncAcceptImpl() override;

	private:

		boost::asio::local::stream_protocol::endpoint m_endpoint;
		boost::asio::local::stream_protocol::acceptor m_acceptor;
		boost::asio::local::stream_protocol::socket m_socket;
	};
}