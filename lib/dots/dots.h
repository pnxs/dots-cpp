#pragma once
#include <functional>
#include <dots/io/Io.h>

namespace dots
{
	uint32_t add_timer(const pnxs::chrono::Duration& timeout, const std::function<void()>& handler, bool periodic = false);
	void remove_timer(uint32_t id);

	void add_fd_handler(int fileDescriptor, const std::function<void()>& handler);
	void remove_fd_handler(int fileDescriptor);
}