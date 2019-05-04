#include <dots/dots.h>
#include <dots/io/Io.h>
#include <dots/io/services/TimerService.h>
#include <dots/io/services/FdHandlerService.h>

namespace dots
{
	uint32_t add_timer(const pnxs::chrono::Duration& timeout, const std::function<void()>& handler, bool periodic/* = false*/)
	{
		return asio::use_service<TimerService>(global_execution_context()).addTimer(timeout, handler, periodic);
	}

	void remove_timer(uint32_t id)
	{
		asio::use_service<TimerService>(global_execution_context()).removeTimer(id);
	}

	void add_fd_handler(int fileDescriptor, const std::function<void()>& handler)
	{
		asio::use_service<FdHandlerService>(global_execution_context()).addInEventHandler(fileDescriptor, handler);
	}

	void remove_fd_handler(int fileDescriptor)
	{
		asio::use_service<FdHandlerService>(global_execution_context()).removeInEventHandler(fileDescriptor);
	}
}