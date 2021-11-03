#pragma once
#include <optional>
#include <boost/asio.hpp>
#include <dots/io/Listener.h>
#include <dots/io/channels/TcpChannel.h>

namespace dots::io
{
    struct TcpListener : Listener
    {
        TcpListener(boost::asio::io_context& ioContext, const Endpoint& endpoint, std::optional<int> backlog = std::nullopt);
        TcpListener(boost::asio::io_context& ioContext, std::string address, std::string port, std::optional<int> backlog = std::nullopt);
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
        boost::asio::ip::tcp::acceptor m_acceptor;
        boost::asio::ip::tcp::socket m_socket;
    };
}