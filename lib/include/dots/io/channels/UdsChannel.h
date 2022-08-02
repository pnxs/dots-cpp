// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <dots/asio.h>
#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
#include <dots/io/channels/AsyncStreamChannel.h>

namespace dots::io::posix::details
{
    template <typename Serializer, TransmissionFormat TransmissionFormat>
    struct GenericUdsChannel : AsyncStreamChannel<asio::local::stream_protocol::socket, Serializer, TransmissionFormat>
    {
        using base_t = AsyncStreamChannel<asio::local::stream_protocol::socket, Serializer, TransmissionFormat>;
        using key_t = typename base_t::key_t;
        using payload_cache_t = typename base_t::payload_cache_t;

        GenericUdsChannel(key_t key, asio::io_context& ioContext, const Endpoint& endpoint);
        GenericUdsChannel(key_t key, asio::io_context& ioContext, std::string_view path);
        GenericUdsChannel(key_t key, asio::local::stream_protocol::socket&& socket, payload_cache_t* payloadCache);
        GenericUdsChannel(const GenericUdsChannel& other) = delete;
        GenericUdsChannel(GenericUdsChannel&& other) = delete;
        virtual ~GenericUdsChannel() noexcept = default;

        GenericUdsChannel& operator = (const GenericUdsChannel& rhs) = delete;
        GenericUdsChannel& operator = (GenericUdsChannel&& rhs) = delete;

    private:

        using stream_t = typename base_t::stream_t;

        using base_t::stream;
        using base_t::initEndpoints;

        static void IgnorePipeSignals();
    };

    extern template struct GenericUdsChannel<serialization::CborSerializer, TransmissionFormat::v1>;
    extern template struct GenericUdsChannel<serialization::CborSerializer, TransmissionFormat::v2>;
}

namespace dots::io::posix
{
    namespace v1
    {
        using UdsChannel = details::GenericUdsChannel<serialization::CborSerializer, TransmissionFormat::v1>;
    }

    inline namespace v2
    {
        using UdsChannel = details::GenericUdsChannel<serialization::CborSerializer, TransmissionFormat::v2>;
    }
}

#else
#error "Local sockets are not available on this platform"
#endif
