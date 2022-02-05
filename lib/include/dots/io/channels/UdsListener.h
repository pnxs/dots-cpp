#pragma once
#include <dots/asio.h>
#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
#include <string_view>
#include <optional>
#include <dots/io/Listener.h>
#include <dots/io/channels/UdsChannel.h>

namespace dots::io::posix
{
    struct UdsListener : Listener
    {
        UdsListener(asio::io_context& ioContext, const Endpoint& endpoint, std::optional<int> backlog = std::nullopt);
        UdsListener(asio::io_context& ioContext, std::string_view path, std::optional<int> backlog = std::nullopt);
        UdsListener(const UdsListener& other) = delete;
        UdsListener(UdsListener&& other) = delete;
        ~UdsListener();

        UdsListener& operator = (const UdsListener& rhs) = delete;
        UdsListener& operator = (UdsListener&& rhs) = delete;

    protected:

        void asyncAcceptImpl() override;

    private:

        asio::local::stream_protocol::endpoint m_endpoint;
        asio::local::stream_protocol::acceptor m_acceptor;
        asio::local::stream_protocol::socket m_socket;
        UdsChannel::payload_cache_t m_payloadCache;
    };
}
#else
#error "Local sockets are not available on this platform"
#endif
