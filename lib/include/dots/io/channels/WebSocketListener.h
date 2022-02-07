#pragma once
#include <dots/asio.h>
#include <boost/beast.hpp>
#include <dots/io/Listener.h>

namespace dots::io
{
    struct WebSocketListener : Listener
    {
        WebSocketListener(asio::io_context& ioContext, const Endpoint& endpoint, std::optional<int> backlog = std::nullopt);
        WebSocketListener(asio::io_context& ioContext, std::string address, std::string port, std::optional<int> backlog = std::nullopt);
        WebSocketListener(const WebSocketListener& other) = delete;
        WebSocketListener(WebSocketListener&& other) = delete;
        ~WebSocketListener() override = default;

        WebSocketListener& operator = (const WebSocketListener& rhs) = delete;
        WebSocketListener& operator = (WebSocketListener&& rhs) = delete;

    protected:

        void asyncAcceptImpl() override;

    private:

        std::string m_address;
        std::string m_port;
        asio::ip::tcp::acceptor m_acceptor;
        asio::ip::tcp::socket m_socket;
    };
}
