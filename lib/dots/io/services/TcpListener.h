#pragma once
#include <functional>
#include <asio.hpp>
#include <dots/io/services/Listener.h>
#include <dots/io/services/TcpChannel.h>

namespace dots
{
	struct TcpListener : Listener
	{
		TcpListener(asio::io_context& ioContext, std::string address, std::string port, int backlog);
		TcpListener(const TcpListener& other) = delete;
		TcpListener(TcpListener&& other) = delete;
		~TcpListener() = default;

		TcpListener& operator = (const TcpListener& rhs) = delete;
		TcpListener& operator = (TcpListener&& rhs) = delete;

		void asyncAccept(std::function<void(channel_ptr_t)>&& handler) override;

	private:

		std::string m_address;
		std::string m_port;
		asio::ip::tcp::acceptor m_acceptor;
		asio::ip::tcp::socket m_socket;
	};
}