#pragma once
#include <string_view>
#include <optional>
#include <asio.hpp>
#include <dots/io/services/Listener.h>
#include <dots/io/services/UdsChannel.h>

namespace dots::io::posix
{
	struct UdsListener : Listener
	{
		UdsListener(asio::io_context& ioContext, const std::string_view& path, std::optional<int> backlog = std::nullopt);
		UdsListener(const UdsListener& other) = delete;
		UdsListener(UdsListener&& other) = delete;
		~UdsListener();

		UdsListener& operator = (const UdsListener& rhs) = delete;
		UdsListener& operator = (UdsListener&& rhs) = delete;

	protected:

		void asyncAcceptImpl() override;

	private:

		asio::local::stream_protocol::endpoint m_endpoint;
		asio::local::stream_protocol::acceptor m_acceptor;
		asio::local::stream_protocol::socket m_socket;
	};
}