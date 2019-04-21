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

	DotsSocketPtr TcpService::connect(const std::string& host, int port)
	{
		auto socket = std::make_shared<TcpSocket>(asio::ip::tcp::socket{ static_cast<asio::io_context&>(context()) });
		
		if (!socket->connect(host, port))
		{
			throw std::runtime_error{ "unable to connect to host: " + host + ":" + std::to_string(port) };
		}

		return socket;
	}

	void TcpService::shutdown() noexcept
	{
		/* do nothing */
	}
}