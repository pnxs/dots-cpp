#include "UdsListener.h"

namespace dots::io::posix
{
	UdsListener::UdsListener(asio::io_context& ioContext, const std::string_view& path, std::optional<int> backlog/* = std::nullopt*/) :
		m_endpoint{ path.data() },
		m_acceptor{ ioContext },
		m_socket{ ioContext }
	{
		m_acceptor.open(m_endpoint.protocol());
		m_acceptor.set_option(asio::local::stream_protocol::acceptor::reuse_address(true));
		m_acceptor.bind(m_endpoint);
		
		if (backlog == std::nullopt)
		{
			m_acceptor.listen();
		}
		else
		{
			m_acceptor.listen(*backlog);
		}
	}

    UdsListener::~UdsListener()
    {
		::unlink(m_endpoint.path().data());
    }

    void UdsListener::asyncAcceptImpl()
	{
		m_acceptor.async_accept(m_socket, [this](const asio::error_code& error)
		{
			if (!m_acceptor.is_open())
			{
				return;
			}

			if (error)
			{
				processError(std::runtime_error{ "failed listening on UDS endpoint at " + m_endpoint.path() + " -> " + error.message() });
				return;
			}

			try
			{
				m_socket.non_blocking(true);

				constexpr int MinimumSendBufferSize = 1024 * 1024;
				asio::socket_base::send_buffer_size sendBufferSize;
				m_socket.get_option(sendBufferSize);

				if (sendBufferSize.value() < MinimumSendBufferSize)
				{
					m_socket.set_option(asio::socket_base::send_buffer_size(MinimumSendBufferSize));
				}

				// note: this move is explicitly allowed according to the ASIO v1.12.2 documentation of the socket
				processAccept(std::make_shared<UdsChannel>(std::move(m_socket)));
			}
			catch (const std::exception& e)
			{
				try
				{
					processError(std::string{ "failed to configure UDS socket -> " } + e.what());

					m_socket.shutdown(asio::local::stream_protocol::socket::shutdown_both);
					m_socket.close();
				}
				catch (const std::exception& e)
				{
					processError(std::string{ "failed to shutdown and close UDS socket -> " } + e.what());
				}
			}
		});
	}
}