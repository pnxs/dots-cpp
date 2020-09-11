#include <dots/io/channels/TcpListener.h>

namespace dots::io
{
    TcpListener::TcpListener(boost::asio::io_context& ioContext, std::string address, std::string port, std::optional<int> backlog/* = std::nullopt*/) :
        m_address{ std::move(address) },
        m_port{ std::move(port) },
        m_acceptor{ ioContext },
        m_socket{ ioContext }
    {
        try
        {
            boost::asio::ip::tcp::resolver resolver{ ioContext };
            boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve({ m_address, m_port });

            m_acceptor.open(endpoint.protocol());
            m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
            m_acceptor.bind(endpoint);

            if (backlog == std::nullopt)
            {
                m_acceptor.listen();
            }
            else
            {
                m_acceptor.listen(*backlog);
            }
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error{ "failed creating TCP listener at address '" + m_address + ":" + m_port + "' -> " + e.what() };
        }
    }

    void TcpListener::asyncAcceptImpl()
    {
        m_acceptor.async_accept(m_socket, [this](const boost::system::error_code& error)
        {
            if (!m_acceptor.is_open())
            {
                return;
            }

            if (error)
            {
                processError(std::make_exception_ptr(std::runtime_error{ "failed listening on TCP endpoint at address '" + m_address + ":" + m_port + "' -> " + error.message() }));
                return;
            }

            try
            {
                m_socket.non_blocking(true);
                m_socket.set_option(boost::asio::ip::tcp::no_delay(true));
                m_socket.set_option(boost::asio::ip::tcp::socket::keep_alive(true));

                constexpr int MinimumSendBufferSize = 1024 * 1024;
                boost::asio::socket_base::send_buffer_size sendBufferSize;
                m_socket.get_option(sendBufferSize);

                if (sendBufferSize.value() < MinimumSendBufferSize)
                {
                    m_socket.set_option(boost::asio::socket_base::send_buffer_size(MinimumSendBufferSize));
                }

                // note: this move is explicitly allowed according to the Boost ASIO v1.72 documentation of the socket
                processAccept(make_channel<TcpChannel>(std::move(m_socket)));
            }
            catch (const std::exception& e)
            {
                try
                {
                    processError(std::string{ "failed to configure TCP socket -> " } + e.what());

                    m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                    m_socket.close();
                }
                catch (const std::exception& e)
                {
                    processError(std::string{ "failed to shutdown and close TCP socket -> " } + e.what());
                }
            }
        });
    }
}