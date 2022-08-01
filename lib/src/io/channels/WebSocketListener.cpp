// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/io/channels/WebSocketListener.h>
#include <algorithm>
#include <dots/io/channels/WebSocketChannel.h>

namespace dots::io
{
    WebSocketListener::WebSocketListener(asio::io_context& ioContext, const Endpoint& endpoint, std::optional<int> backlog/* = std::nullopt*/) :
        WebSocketListener(ioContext, std::string{ endpoint.host() }, std::string{ endpoint.port() }, backlog)
    {
        /* do nothing */
    }

    WebSocketListener::WebSocketListener(asio::io_context& ioContext, std::string address, std::string port, std::optional<int> backlog/* = std::nullopt*/) :
        m_address{ std::move(address) },
        m_port{ std::move(port) },
        m_acceptor{ ioContext },
        m_socket{ ioContext }
    {
        try
        {
            boost::beast::net::ip::tcp::resolver resolver{ ioContext };
            boost::beast::net::ip::tcp::endpoint endpoint = *resolver.resolve({ m_address, m_port });

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
            throw std::runtime_error{ "failed creating WebSocket listener at address '" + m_address + ":" + m_port + "' -> " + e.what() };
        }
    }

    void WebSocketListener::asyncAcceptImpl()
    {
        m_acceptor.async_accept(m_socket, [this](const boost::system::error_code& error)
        {
            try
            {
                if (error == asio::error::operation_aborted || !m_acceptor.is_open())
                {
                    return;
                }

                if (error)
                {
                    processError(std::make_exception_ptr(std::runtime_error{ "failed listening on WebSocket endpoint at address '" + m_address + ":" + m_port + "' -> " + error.message() }));
                    return;
                }

                // note: this move is explicitly allowed according to the Boost ASIO v1.72 documentation of the socket
                WebSocketChannel::ws_stream_t stream{ std::move(m_socket) };

                stream.set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::server));
                stream.set_option(boost::beast::websocket::stream_base::decorator([](boost::beast::websocket::response_type& res)
                {
                    res.set(boost::beast::http::field::server, std::string(BOOST_BEAST_VERSION_STRING) + " DOTS WebSocket server");
                    res.set(boost::beast::http::field::sec_websocket_protocol, WebSocketChannel::Subprotocol);
                }));

                stream.accept();

                processAccept(make_channel<WebSocketChannel>(std::move(stream)));
            }
            catch (const std::exception& e)
            {
                try
                {
                    processError(std::string{ "failed to configure WebSocket stream -> " } + e.what());

                    m_socket.shutdown(asio::ip::tcp::socket::shutdown_both);
                    m_socket.close();
                }
                catch (const std::exception& e)
                {
                    processError(std::string{ "failed to shutdown and close WebSocket stream -> " } + e.what());
                }
            }
        });
    }
}
