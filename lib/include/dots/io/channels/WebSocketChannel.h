// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <string_view>
#include <dots/asio.h>
#include <boost/beast.hpp>
#include <dots/io/Channel.h>
#include <dots/serialization/JsonSerializer.h>

namespace dots::io
{
    struct WebSocketChannel : Channel
    {
        using ws_stream_t = boost::beast::websocket::stream<boost::beast::tcp_stream>;
        static constexpr char Subprotocol[] = "dots-json";

        WebSocketChannel(key_t key, asio::io_context& ioContext, const Endpoint& endpoint);
        WebSocketChannel(key_t key, asio::io_context& ioContext, std::string_view host, std::string_view port);
        WebSocketChannel(key_t key, ws_stream_t&& stream);
        WebSocketChannel(const WebSocketChannel& other) = delete;
        WebSocketChannel(WebSocketChannel&& other) = delete;
        ~WebSocketChannel() override = default;

        WebSocketChannel& operator = (const WebSocketChannel& rhs) = delete;
        WebSocketChannel& operator = (WebSocketChannel&& rhs) = delete;

    protected:

        void asyncReceiveImpl() override;
        void transmitImpl(const DotsHeader& header, const type::Struct& instance) override;

    private:

        ws_stream_t m_stream;
        boost::beast::flat_buffer m_buffer;
        serialization::JsonSerializer m_serializer;
    };
}
