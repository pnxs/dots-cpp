#include <boost/asio.hpp>
#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
#include <dots/io/channels/UdsChannel.h>
#include <csignal>

namespace dots::io::posix
{
    UdsChannel::UdsChannel(Channel::key_t key, boost::asio::io_context& ioContext, const Endpoint& endpoint) :
        UdsChannel(key, ioContext, endpoint.path())
    {
        /* do nothing */
    }

    UdsChannel::UdsChannel(Channel::key_t key, boost::asio::io_context& ioContext, std::string_view path) :
        AsyncStreamChannel(key, stream_t{ ioContext }, nullptr)
    {
        try
        {
            stream().connect(std::string{ path });
            initEndpoints(Endpoint{ "uds", stream().local_endpoint().path() }, Endpoint{ "uds", stream().local_endpoint().path() });
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error{ "could not open UDS connection '" + std::string{ path } + "': " + e.what() };
        }

        IgnorePipeSignals();
    }

    UdsChannel::UdsChannel(Channel::key_t key, boost::asio::local::stream_protocol::socket&& socket_, payload_cache_t* payloadCache) :
        AsyncStreamChannel(key, std::move(socket_), payloadCache)
    {
        IgnorePipeSignals();

        if (stream().is_open())
        {
            initEndpoints(Endpoint{ "uds", stream().local_endpoint().path() }, Endpoint{ "uds", stream().local_endpoint().path() });
        }
    }

    void UdsChannel::IgnorePipeSignals()
    {
        // ignores all pipe signals to prevent non-catchable application termination on broken pipes
        static auto IgnorePipesSignals = [](){ return std::signal(SIGPIPE, SIG_IGN); }();
        (void)IgnorePipesSignals;
    }
}
#endif
