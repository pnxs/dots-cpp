#pragma once

#include "dots/cpp_config.h"
#include <dots/common/seconds.h>
#include <dots/common/Chrono.h>
#include "Timer.h"
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

namespace dots {

class AsioSingleShotTimer
{
	using timer_t = boost::asio::steady_timer;
	using duration_t = timer_t::clock_type::duration;

	timer_t m_timer;
    const function<void ()> m_cb;
    unsigned int m_id;
    pnxs::Duration m_interval;
    pnxs::SteadyTimePoint m_next;
    bool m_periodic;

	inline static unsigned int m_lastTimerId = 1;

    void callCb();
	void onTimeout(const boost::system::error_code& error);


public:
    AsioSingleShotTimer(const pnxs::Duration & interval, const function<void ()> &cb, bool periodic = false);
    ~AsioSingleShotTimer();

    inline static std::map<unsigned int, AsioSingleShotTimer*> s_all;

	void startRelative(const pnxs::Duration& duration);
	void startAbsolute(const pnxs::SteadyTimePoint& timepoint);

    unsigned int id() { return m_id; }
    static void remTimer(unsigned int id);
};


}
