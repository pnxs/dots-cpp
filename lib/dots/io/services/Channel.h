#pragma once
#include <string>
#include <vector>
#include <dots/type/AnyStruct.h>
#include <DotsTransportHeader.dots.h>

namespace dots
{
	struct Channel
	{
		using receive_handler_t = std::function<void(const DotsTransportHeader&, type::AnyStruct&&, const std::vector<uint8_t>&)>;
		using error_handler_t = std::function<void(int)>;

		Channel() = default;
		Channel(const Channel& other) = delete;
		Channel(Channel&& other) = delete;
		virtual ~Channel() = default;

		Channel& operator = (const Channel& rhs) = delete;
		Channel& operator = (Channel&& rhs) = delete;

		virtual void asyncReceive(receive_handler_t&& receiveHandler, error_handler_t&& errorHandler) = 0;
		virtual int transmit(const DotsTransportHeader& header, const type::Struct& instance) = 0;
	};

	using channel_ptr_t = std::shared_ptr<Channel>;
}