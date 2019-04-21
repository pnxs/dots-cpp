#include "TcpListener.h"
#include <dots/common/logging.h>

namespace dots
{
	TcpListener::TcpListener(asio::io_context& ioContext, std::string address, std::string port, int backlog) :
		m_address{ std::move(address) },
		m_port{ std::move(port) },
		m_acceptor{ ioContext },
		m_socket{ ioContext }
	{
		asio::ip::tcp::resolver resolver{ ioContext };
		asio::ip::tcp::endpoint endpoint = *resolver.resolve({ m_address, m_port });

		m_acceptor.open(endpoint.protocol());
		m_acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
		m_acceptor.bind(endpoint);
		m_acceptor.listen(backlog);
	}

	void TcpListener::asyncAccept(std::function<void(ChannelPtr)>&& handler)
	{
		m_acceptor.async_accept(m_socket, [this, handler = std::move(handler)](const asio::error_code& error)
		{
			if (!m_acceptor.is_open())
			{
				return;
			}

			if (error)
			{
				LOG_WARN_S("failed listening on TCP endpoint at " << m_address << ":" << m_port << " -> " << error.message());
				return;
			}

			try
			{
				m_socket.non_blocking(true);
				m_socket.set_option(asio::ip::tcp::no_delay(true));
				m_socket.set_option(asio::ip::tcp::socket::keep_alive(true));

				constexpr int MinimumSendBufferSize = 1024 * 1024;
				asio::socket_base::send_buffer_size sendBufferSize;
				m_socket.get_option(sendBufferSize);

				if (sendBufferSize.value() < MinimumSendBufferSize)
				{
					m_socket.set_option(asio::socket_base::send_buffer_size(MinimumSendBufferSize));
				}

				// note: this move is explicitly allowed according to the ASIO v1.12.2 documentation of the socket
				handler(std::make_shared<TcpChannel>(std::move(m_socket)));
			}
			catch (const std::exception & e)
			{
				LOG_WARN_S("failed to configure TCP socket -> " << e.what());
			}
		});
	}
}