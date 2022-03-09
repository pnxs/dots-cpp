#pragma once
#include <dots/asio.h>
#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
#include <string_view>
#include <optional>
#include <dots/io/Listener.h>
#include <dots/io/channels/UdsChannel.h>

namespace dots::io::posix::details
{
    template <typename TChannel>
    struct GenericUdsListener : Listener
    {
        GenericUdsListener(asio::io_context& ioContext, const Endpoint& endpoint, std::optional<int> backlog = std::nullopt);
        GenericUdsListener(asio::io_context& ioContext, std::string_view path, std::optional<int> backlog = std::nullopt);
        GenericUdsListener(const GenericUdsListener& other) = delete;
        GenericUdsListener(GenericUdsListener&& other) = delete;
        ~GenericUdsListener();

        GenericUdsListener& operator = (const GenericUdsListener& rhs) = delete;
        GenericUdsListener& operator = (GenericUdsListener&& rhs) = delete;

    protected:

        void asyncAcceptImpl() override;

    private:

        using buffer_t = typename TChannel::buffer_t;
        using payload_cache_t = typename TChannel::payload_cache_t;

        asio::local::stream_protocol::endpoint m_endpoint;
        asio::local::stream_protocol::acceptor m_acceptor;
        asio::local::stream_protocol::socket m_socket;
        payload_cache_t m_payloadCache;
    };

    extern template struct GenericUdsListener<LegacyUdsChannel>;
    extern template struct GenericUdsListener<UdsChannel>;
}

namespace dots::io::posix
{
    using LegacyUdsListener = details::GenericUdsListener<LegacyUdsChannel>;
    using UdsListener = details::GenericUdsListener<UdsChannel>;
}

#else
#error "Local sockets are not available on this platform"
#endif
