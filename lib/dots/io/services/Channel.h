#pragma once
#include <cstdint>
#include <dots/cpp_config.h>
#include <DotsTransportHeader.dots.h>
#include <dots/io/Message.h>

namespace dots
{
	struct Channel
	{
		using receive_callback = std::function<void(const Message&)>;
		using error_callback = std::function<void(int ec)>;

		virtual ~Channel() = default;

		virtual void start() = 0;

		virtual void setReceiveCallback(receive_callback cb) = 0;
		virtual void setErrorCallback(error_callback cb) = 0;

		virtual int send(const DotsTransportHeader& header, const std::vector<uint8_t>& data = {}) = 0;

		virtual bool connect(const std::string& host, int port) = 0;
		virtual void disconnect() = 0;
	};

	typedef shared_ptr<Channel> ChannelPtr;
}