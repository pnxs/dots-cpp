#pragma once
#include <optional>
#include <dots/asio.h>
#include <dots/io/Listener.h>
#include <dots/io/channels/TcpChannel.h>

namespace dots::io
{
    struct TcpListener : Listener
    {
        TcpListener(asio::io_context& ioContext, const Endpoint& endpoint, std::optional<int> backlog = std::nullopt);
        TcpListener(asio::io_context& ioContext, std::string address, std::string port, std::optional<int> backlog = std::nullopt);
        TcpListener(const TcpListener& other) = delete;
        TcpListener(TcpListener&& other) = delete;
        ~TcpListener() override = default;

        TcpListener& operator = (const TcpListener& rhs) = delete;
        TcpListener& operator = (TcpListener&& rhs) = delete;

    protected:

        void asyncAcceptImpl() override;

    private:

        std::string m_address;
        std::string m_port;
        asio::ip::tcp::acceptor m_acceptor;
        asio::ip::tcp::socket m_socket;
        TcpChannel::payload_cache_t m_payloadCache;
    };
}
