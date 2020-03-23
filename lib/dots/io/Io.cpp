#include <dots/io/Io.h>
#include <dots/io/services/TimerService.h>
#include <dots/io/services/FdHandlerService.h>

namespace dots
{
	boost::asio::io_context& global_io_context()
	{
		static boost::asio::io_context ioContext;
		return ioContext;
	}

	boost::asio::execution_context& global_execution_context()
	{
		return static_cast<boost::asio::execution_context&>(global_io_context());
	}

	boost::asio::executor global_executor()
	{
		return global_io_context().get_executor();
	}
}