#pragma once
#include <functional>
#include <dots/common/Chrono.h>
#include <dots/io/services/Timer.h>

namespace dots
{
	Timer::id_t add_timer(const pnxs::chrono::Duration& timeout, const std::function<void()>& handler, bool periodic = false);
	void remove_timer(Timer::id_t id);

	void add_fd_handler(int fileDescriptor, const std::function<void()>& handler);
	void remove_fd_handler(int fileDescriptor);
}