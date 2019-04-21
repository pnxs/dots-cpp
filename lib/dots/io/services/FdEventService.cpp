#include "FdEventService.h"

namespace dots
{
	FdEventService::FdEventService(asio::execution_context& executionContext) :
		asio::execution_context::service(executionContext)
	{
		/* do nothing */
	}

	void FdEventService::addInEventHandler(int fileDescriptor, const callback_t& callback)
	{
		m_inEventHandlers.try_emplace(fileDescriptor, static_cast<asio::io_context&>(context()), fileDescriptor, callback);
	}

	void FdEventService::removeInEventHandler(int fileDescriptor)
	{
		if (auto it = m_inEventHandlers.find(fileDescriptor);  it != m_inEventHandlers.end())
		{
			m_inEventHandlers.erase(it);
		}
	}

	void FdEventService::shutdown() noexcept
	{
		m_inEventHandlers.clear();
	}
}