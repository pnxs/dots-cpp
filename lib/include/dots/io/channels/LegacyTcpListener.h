#pragma once
#include <optional>
#include <boost/asio.hpp>
#include <dots/io/Listener.h>
#include <dots/io/channels/LegacyTcpChannel.h>

namespace dots::io
{
    struct LegacyTcpListener : Listener
    {
        LegacyTcpListener(boost::asio::io_context& ioContext, const Endpoint& endpoint, std::optional<int> backlog = std::nullopt);
        LegacyTcpListener(boost::asio::io_context& ioContext, std::string address, std::string port, std::optional<int> backlog = std::nullopt);
        LegacyTcpListener(const LegacyTcpListener& other) = delete;
        LegacyTcpListener(LegacyTcpListener&& other) = delete;
        ~LegacyTcpListener() override = default;

        LegacyTcpListener& operator = (const LegacyTcpListener& rhs) = delete;
        LegacyTcpListener& operator = (LegacyTcpListener&& rhs) = delete;

    protected:

        void asyncAcceptImpl() override;

    private:

        std::string m_address;
        std::string m_port;
        boost::asio::ip::tcp::acceptor m_acceptor;
        boost::asio::ip::tcp::socket m_socket;
        LegacyTcpChannel::payload_cache_t m_payloadCache;
    };
}
