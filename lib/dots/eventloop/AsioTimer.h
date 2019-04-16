#pragma once

#include "dots/cpp_config.h"
#include <dots/common/seconds.h>
#include <dots/common/Chrono.h>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

namespace dots
{
	struct AsioTimer
	{
		using timer_id_t = uint32_t;
		using callback_t = std::function<void()>;

		AsioTimer(boost::asio::io_service& ioService, const pnxs::Duration& interval, const callback_t& cb, bool periodic = false);
		AsioTimer(const AsioTimer& other) = delete;
		AsioTimer(AsioTimer&& other) = delete;
		~AsioTimer();

		AsioTimer& operator = (const AsioTimer& rhs) = delete;
		AsioTimer& operator = (AsioTimer&& rhs) = delete;

		timer_id_t id() { return m_id; }
		static void remTimer(unsigned int id);

	private:		

		using timer_t = boost::asio::steady_timer;
		using duration_t = timer_t::clock_type::duration;

		void callCb();
		void onTimeout(const boost::system::error_code& error);
		void startRelative(const pnxs::Duration& duration);
		void startAbsolute(const pnxs::SteadyTimePoint& timepoint);

		inline static timer_id_t m_lastTimerId = 1;
		inline static std::map<unsigned int, AsioTimer*> s_all;

		timer_t m_timer;
		callback_t m_cb;
		timer_id_t m_id;
		pnxs::Duration m_interval;
		pnxs::SteadyTimePoint m_next;
		bool m_periodic;			
	};
}