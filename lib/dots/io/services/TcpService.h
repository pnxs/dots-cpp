#pragma once
#include <asio.hpp>
#include <dots/io/services/TcpSocket.h>

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

		DotsSocketPtr connect(const std::string& host, int port);

	private:

		void shutdown() noexcept override;

	};
}