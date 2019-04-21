#include "TcpService.h"

namespace dots
{
	TcpService::TcpService(asio::execution_context& executionContext) :
		asio::execution_context::service(executionContext)
	{
		/* do nothing */
	}

	std::unique_ptr<Listener> TcpService::listen(const std::string& address, const std::string& port, int backlog)
	{
		return std::make_unique<TcpListener>(static_cast<asio::io_context&>(context()), address, port, backlog);
	}

	ChannelPtr TcpService::connect(const std::string& host, int port)
	{
		auto channel = std::make_shared<TcpChannel>(static_cast<asio::io_context&>(context()), host, port);
		return channel;
	}

	void TcpService::shutdown() noexcept
	{
		/* do nothing */
	}
}