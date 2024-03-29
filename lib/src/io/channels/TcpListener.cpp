// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/io/channels/TcpListener.h>

namespace dots::io::details
{
    template <typename TChannel>
    GenericTcpListener<TChannel>::GenericTcpListener(asio::io_context& ioContext, const Endpoint& endpoint, std::optional<int> backlog/* = std::nullopt*/) :
        GenericTcpListener(ioContext, std::string{ endpoint.host() }, std::string{ endpoint.port().empty() ? DefaultPort : endpoint.port() }, backlog)
    {
        /* do nothing */
    }

    template <typename TChannel>
    GenericTcpListener<TChannel>::GenericTcpListener(asio::io_context& ioContext, std::string address, std::string port, std::optional<int> backlog/* = std::nullopt*/) :
        m_address{ std::move(address) },
        m_port{ std::move(port) },
        m_acceptor{ ioContext },
        m_socket{ ioContext },
        m_payloadCache{ 0, buffer_t{} }
    {
        try
        {
            asio::ip::tcp::resolver resolver{ ioContext };
            asio::ip::tcp::endpoint endpoint = *resolver.resolve({ m_address, m_port });

            m_acceptor.open(endpoint.protocol());
            m_acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
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

    template <typename TChannel>
    void GenericTcpListener<TChannel>::asyncAcceptImpl()
    {
        m_acceptor.async_accept(m_socket, [this](const boost::system::error_code& error)
        {
            if (error == asio::error::operation_aborted || !m_acceptor.is_open())
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
                m_socket.set_option(asio::ip::tcp::no_delay(true));
                m_socket.set_option(asio::ip::tcp::socket::keep_alive(true));

                constexpr int MinimumSendBufferSize = 1024 * 1024;
                asio::socket_base::send_buffer_size sendBufferSize;
                m_socket.get_option(sendBufferSize);

                if (sendBufferSize.value() < MinimumSendBufferSize)
                {
                    m_socket.set_option(asio::socket_base::send_buffer_size(MinimumSendBufferSize));
                }

                // note: this move is explicitly allowed according to the Boost ASIO v1.72 documentation of the socket
                processAccept(make_channel<TChannel>(std::move(m_socket), &m_payloadCache));
            }
            catch (const std::exception& e)
            {
                try
                {
                    processError(std::string{ "failed to configure TCP socket -> " } + e.what());

                    m_socket.shutdown(asio::ip::tcp::socket::shutdown_both);
                    m_socket.close();
                }
                catch (const std::exception& e)
                {
                    processError(std::string{ "failed to shutdown and close TCP socket -> " } + e.what());
                }
            }
        });
    }

    template struct GenericTcpListener<v1::TcpChannel>;
    template struct GenericTcpListener<v2::TcpChannel>;
}
