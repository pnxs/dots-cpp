// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/asio.h>
#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
#include <dots/io/channels/UdsChannel.h>
#include <csignal>

namespace dots::io::posix::details
{
    template <typename Serializer, TransmissionFormat TransmissionFormat>
    GenericUdsChannel<Serializer, TransmissionFormat>::GenericUdsChannel(key_t key, asio::io_context& ioContext, const Endpoint& endpoint) :
        GenericUdsChannel(key, ioContext, endpoint.path())
    {
        /* do nothing */
    }

    template <typename Serializer, TransmissionFormat TransmissionFormat>
    GenericUdsChannel<Serializer, TransmissionFormat>::GenericUdsChannel(key_t key, asio::io_context& ioContext, std::string_view path) :
        base_t(key, stream_t{ ioContext }, nullptr)
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

    template <typename Serializer, TransmissionFormat TransmissionFormat>
    GenericUdsChannel<Serializer, TransmissionFormat>::GenericUdsChannel(key_t key, asio::local::stream_protocol::socket&& socket_, payload_cache_t* payloadCache) :
        base_t(key, std::move(socket_), payloadCache)
    {
        IgnorePipeSignals();

        if (stream().is_open())
        {
            initEndpoints(Endpoint{ "uds", stream().local_endpoint().path() }, Endpoint{ "uds", stream().local_endpoint().path() });
        }
    }

    template <typename Serializer, TransmissionFormat TransmissionFormat>
    void GenericUdsChannel<Serializer, TransmissionFormat>::IgnorePipeSignals()
    {
        // ignores all pipe signals to prevent non-catchable application termination on broken pipes
        static auto IgnorePipesSignals = [](){ return std::signal(SIGPIPE, SIG_IGN); }();
        (void)IgnorePipesSignals;
    }

    template struct GenericUdsChannel<serialization::CborSerializer, TransmissionFormat::v1>;
    template struct GenericUdsChannel<serialization::CborSerializer, TransmissionFormat::v2>;
}
#endif
