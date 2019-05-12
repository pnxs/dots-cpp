#pragma once
#include <functional>
#include <asio.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <dots/io/services/Listener.h>

namespace dots
{
	struct WebSocketListener : Listener
	{
		WebSocketListener(asio::io_context& ioContext, uint16_t port);
		WebSocketListener(const WebSocketListener& other) = delete;
		WebSocketListener(WebSocketListener&& other) = delete;
		~WebSocketListener() = default;

		WebSocketListener& operator = (const WebSocketListener& rhs) = delete;
		WebSocketListener& operator = (WebSocketListener&& rhs) = delete;

		void asyncAccept(std::function<void(channel_ptr_t)>&& handler) override;

	private:

        using ws_server_t = websocketpp::server<websocketpp::config::asio>;
        using ws_connection_ptr_t = ws_server_t::connection_ptr;
		using ws_connection_hdl_t = websocketpp::connection_hdl;

		ws_server_t m_wsServer;
	};
}