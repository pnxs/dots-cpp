#pragma once
#include <boost/asio.hpp>
#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
#include <dots/io/channels/AsyncStreamChannel.h>

namespace dots::io::posix
{
    struct UdsChannel : AsyncStreamChannel<boost::asio::local::stream_protocol::socket>
    {
        UdsChannel(Channel::key_t key, boost::asio::io_context& ioContext, const Endpoint& endpoint);
        UdsChannel(Channel::key_t key, boost::asio::io_context& ioContext, std::string_view path);
        UdsChannel(Channel::key_t key, boost::asio::local::stream_protocol::socket&& socket, payload_cache_t* payloadCache);
        UdsChannel(const UdsChannel& other) = delete;
        UdsChannel(UdsChannel&& other) = delete;
        virtual ~UdsChannel() noexcept = default;

        UdsChannel& operator = (const UdsChannel& rhs) = delete;
        UdsChannel& operator = (UdsChannel&& rhs) = delete;

    private:

        static void IgnorePipeSignals();
    };
}
#else
#error "Local sockets are not available on this platform"
#endif
