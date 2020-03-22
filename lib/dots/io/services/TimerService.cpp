#include "TimerService.h"

namespace dots
{
	TimerService::TimerService(asio::execution_context& executionContext) : 
		asio::execution_context::service(executionContext)
	{
		/* do nothing */
	}

	auto TimerService::addTimer(const type::Duration& timeout, const callback_t& cb, bool periodic) -> Timer::id_t
	{
		Timer::id_t id = ++m_lastTimerId;
		m_timers.try_emplace(id, static_cast<asio::io_context&>(context()), id, timeout, cb, periodic);

		return id;
	}	

	void TimerService::removeTimer(unsigned id)
	{
		m_timers.erase(id);
	}

	void TimerService::shutdown() noexcept
	{
		m_timers.clear();
	}
}