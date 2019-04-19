#include "AsioTimer.h"
#include <dots/functional/fun.h>
#include <dots/eventloop/AsioEventLoop.h>

namespace dots
{
	AsioTimer::AsioTimer(asio::io_context& ioContext, timer_id_t id, const pnxs::Duration& interval, const callback_t& cb, bool periodic) :
		m_timer(ioContext),
		m_cb(cb),
		m_id(id),
		m_interval(interval),
		m_next(pnxs::SteadyNow{}),
		m_periodic(periodic)
	{
		if (m_periodic)
		{
			startAbsolute(m_next += m_interval);
		}
		else
		{
			startRelative(m_interval);
		}
	}

	AsioTimer::~AsioTimer()
	{
		m_timer.cancel();
	}

	void AsioTimer::startRelative(const pnxs::Duration & duration)
	{
		m_timer.expires_after(std::chrono::duration_cast<duration_t>(duration));
		m_timer.async_wait(FUN(*this, onTimeout));
		}

	void AsioTimer::startAbsolute(const pnxs::SteadyTimePoint & timepoint)
	{
		m_timer.expires_at(std::chrono::time_point_cast<duration_t>(timepoint));
		m_timer.async_wait(FUN(*this, onTimeout));
		}

	void AsioTimer::onTimeout(const asio::error_code& error)
	{
		if (error != asio::error::operation_aborted)
		{
			if (m_periodic)
			{
				m_cb();
				startAbsolute(m_next += m_interval);
			}
			else
			{
				m_cb();
				AsioEventLoop::Instance().removeTimer(m_id);
			}
		}
		}
}