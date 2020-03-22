#include "FdHandlerService.h"

namespace dots
{
	FdHandlerService::FdHandlerService(boost::asio::execution_context& executionContext) :
		boost::asio::execution_context::service(executionContext)
	{
		/* do nothing */
	}

	void FdHandlerService::addInEventHandler(int fileDescriptor, const callback_t& callback)
	{
		m_inEventHandlers.try_emplace(fileDescriptor, static_cast<boost::asio::io_context&>(context()), fileDescriptor, callback);
	}

	void FdHandlerService::removeInEventHandler(int fileDescriptor)
	{
		if (auto it = m_inEventHandlers.find(fileDescriptor);  it != m_inEventHandlers.end())
		{
			m_inEventHandlers.erase(it);
		}
	}

	void FdHandlerService::shutdown() noexcept
	{
		m_inEventHandlers.clear();
	}
}