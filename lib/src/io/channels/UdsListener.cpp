#include <boost/asio.hpp>
#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
#include <dots/io/channels/UdsListener.h>

namespace dots::io::posix
{
    UdsListener::UdsListener(boost::asio::io_context& ioContext, const Endpoint& endpoint, std::optional<int> backlog/* = std::nullopt*/) :
        UdsListener(ioContext, endpoint.path(), backlog)
    {
        /* do nothing */
    }

    UdsListener::UdsListener(boost::asio::io_context& ioContext, std::string_view path, std::optional<int> backlog/* = std::nullopt*/) :
        m_endpoint{ path.data() },
        m_acceptor{ ioContext },
        m_socket{ ioContext },
        m_payloadCache{ 0, UdsChannel::buffer_t{} }
    {
        try
        {
            m_acceptor.open(m_endpoint.protocol());
            m_acceptor.set_option(boost::asio::local::stream_protocol::acceptor::reuse_address(true));
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
        catch (const std::exception& e)
        {
            throw std::runtime_error{ "failed creating UDS listener at path '" + m_endpoint.path() + "' -> " + e.what() };
        }
    }

    UdsListener::~UdsListener()
    {
        ::unlink(m_endpoint.path().data());
    }

    void UdsListener::asyncAcceptImpl()
    {
        m_acceptor.async_accept(m_socket, [this](const boost::system::error_code& error)
        {
            if (!m_acceptor.is_open())
            {
                return;
            }

            if (error)
            {
                processError(std::make_exception_ptr(std::runtime_error{ "failed listening on UDS endpoint at path '" + m_endpoint.path() + "' -> " + error.message() }));
                return;
            }

            try
            {
                m_socket.non_blocking(true);

                constexpr int MinimumSendBufferSize = 1024 * 1024;
                boost::asio::socket_base::send_buffer_size sendBufferSize;
                m_socket.get_option(sendBufferSize);

                if (sendBufferSize.value() < MinimumSendBufferSize)
                {
                    m_socket.set_option(boost::asio::socket_base::send_buffer_size(MinimumSendBufferSize));
                }

                // note: this move is explicitly allowed according to the ASIO v1.72 documentation of the socket
                processAccept(make_channel<UdsChannel>(std::move(m_socket), &m_payloadCache));
            }
            catch (const std::exception& e)
            {
                try
                {
                    processError(std::string{ "failed to configure UDS socket -> " } + e.what());

                    m_socket.shutdown(boost::asio::local::stream_protocol::socket::shutdown_both);
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
#endif
