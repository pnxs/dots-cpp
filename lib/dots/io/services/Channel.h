#pragma once
#include <string>
#include <vector>
#include <DotsTransportHeader.dots.h>
#include <dots/io/Message.h>

namespace dots
{
	struct Channel
	{
		Channel() = default;
		Channel(const Channel& other) = delete;
		Channel(Channel&& other) = delete;
		virtual ~Channel() = default;

		Channel& operator = (const Channel& rhs) = delete;
		Channel& operator = (Channel&& rhs) = delete;

		virtual void asyncReceive(std::function<void(const Message&)>&& receiveHandler, std::function<void(int ec)>&& errorHandler) = 0;
		virtual int transmit(const DotsTransportHeader& header, const std::vector<uint8_t>& data = {}) = 0;
	};

	using ChannelPtr = std::shared_ptr<Channel>;
}