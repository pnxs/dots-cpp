#pragma once
#include <functional>
#include <dots/io/services/Channel.h>

namespace dots
{
	struct Listener
	{
		Listener() = default;
		Listener(const Listener& other) = delete;
		Listener(Listener&& other) = delete;
		virtual ~Listener() = default;

		Listener& operator = (const Listener& rhs) = delete;
		Listener& operator = (Listener&& rhs) = delete;

		virtual void asyncAccept(std::function<void(channel_ptr_t)>&& handler) = 0;
	};
}