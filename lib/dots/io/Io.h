#pragma once
#include <functional>
#include <dots/common/Chrono.h>

namespace asio
{
	class io_context;
	class execution_context;
	class executor;
}

namespace dots
{
	asio::io_context& global_io_context();
	asio::execution_context& global_execution_context();
	asio::executor global_executor();

	uint32_t add_timer(const pnxs::chrono::Duration& timeout, const std::function<void()>& handler, bool periodic = false);
	void remove_timer(uint32_t id);

	void add_fd_handler(int fileDescriptor, const std::function<void()>& handler);
	void remove_fd_handler(int fileDescriptor);
}