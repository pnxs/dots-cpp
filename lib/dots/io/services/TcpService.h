#pragma once
#include <asio.hpp>
#include <dots/io/services/TcpSocket.h>
#include <dots/io/services/TcpListener.h>

namespace dots
{
	struct TcpService : asio::execution_context::service
	{
		using key_type = TcpService;

		explicit TcpService(asio::execution_context& executionContext);
		TcpService(const TcpService& other) = delete;
		TcpService(TcpService&& other) noexcept = default;
		~TcpService() = default;

		TcpService& operator = (const TcpService& rhs) = delete;
		TcpService& operator = (TcpService&& rhs) noexcept = default;

		std::unique_ptr<Listener> listen(const std::string& address, const std::string& port, int backlog);
		ChannelPtr connect(const std::string& host, int port);

	private:

		void shutdown() noexcept override;
	};
}