#include "TcpService.h"

namespace dots
{
	TcpService::TcpService(asio::execution_context& executionContext) :
		asio::execution_context::service(executionContext)
	{
		/* do nothing */
	}

	void TcpService::shutdown() noexcept
	{
		/* do nothing */
	}
}