#include <dots/dots.h>
#include <dots/io/Io.h>
#include <dots/io/services/TimerService.h>
#include <dots/io/services/FdHandlerService.h>

namespace dots
{
	Timer::id_t add_timer(const pnxs::chrono::Duration& timeout, const std::function<void()>& handler, bool periodic/* = false*/)
	{
		return global_service<TimerService>().addTimer(timeout, handler, periodic);
	}

	void remove_timer(Timer::id_t id)
	{
		global_service<TimerService>().removeTimer(id);
	}

	void add_fd_handler(int fileDescriptor, const std::function<void()>& handler)
	{
		global_service<FdHandlerService>().addInEventHandler(fileDescriptor, handler);
	}

	void remove_fd_handler(int fileDescriptor)
	{
		global_service<FdHandlerService>().removeInEventHandler(fileDescriptor);
	}

	Transceiver& transceiver()
	{
		static Transceiver transceiver;
		return transceiver;
	}
}