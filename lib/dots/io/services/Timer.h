#pragma once

#include <dots/common/seconds.h>
#include <dots/common/Chrono.h>
#include <asio.hpp>

namespace dots
{
	struct Timer
	{
		using id_t = uint32_t;
		using callback_t = std::function<void()>;

		Timer(asio::io_context& ioContext, id_t id, const pnxs::Duration& interval, const callback_t& cb, bool periodic = false);
		Timer(const Timer& other) = delete;
		Timer(Timer&& other) = delete;
		~Timer();

		Timer& operator = (const Timer& rhs) = delete;
		Timer& operator = (Timer&& rhs) = delete;

		id_t id() { return m_id; }

	private:		

		using timer_t = asio::steady_timer;
		using duration_t = timer_t::clock_type::duration;

		void callCb();
		void onTimeout(const asio::error_code& error);
		void startRelative(const pnxs::Duration& duration);
		void startAbsolute(const pnxs::SteadyTimePoint& timepoint);

		timer_t m_timer;
		callback_t m_cb;
		id_t m_id;
		pnxs::Duration m_interval;
		pnxs::SteadyTimePoint m_next;
		bool m_periodic;			
	};
}