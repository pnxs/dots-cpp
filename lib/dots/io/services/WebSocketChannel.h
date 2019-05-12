#pragma once
#include <optional>
#include <string_view>
#include <asio.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/client.hpp>
#include <dots/io/services/Channel.h>

namespace dots
{
	struct WebSocketChannel : Channel
	{
        using ws_server_t = websocketpp::server<websocketpp::config::asio>;
		using ws_client_t = websocketpp::client<websocketpp::config::asio>;
        using ws_connection_ptr_t = ws_server_t::connection_ptr;
		using ws_message_ptr_t = ws_server_t::message_ptr;
		using ws_connection_hdl_t = websocketpp::connection_hdl;

        WebSocketChannel(asio::io_context& ioContext, const std::string_view& host, const std::string_view& port);
		WebSocketChannel(ws_connection_ptr_t connection);
		WebSocketChannel(const WebSocketChannel& other) = delete;
		WebSocketChannel(WebSocketChannel&& other) = delete;
		virtual ~WebSocketChannel() = default;

		WebSocketChannel& operator = (const WebSocketChannel& rhs) = delete;
		WebSocketChannel& operator = (WebSocketChannel&& rhs) = delete;

	protected:

		void asyncReceiveImpl() override;
		void transmitImpl(const DotsTransportHeader& header, const type::Struct& instance) override;

	private:

		void handleError(const std::string& what);

		ws_connection_ptr_t m_connection;
		std::optional<ws_client_t> m_client;
	};
}