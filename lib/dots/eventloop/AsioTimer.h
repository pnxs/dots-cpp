#pragma once

#include "dots/cpp_config.h"
#include <dots/common/seconds.h>
#include <dots/common/Chrono.h>
#include "Timer.h"
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

namespace dots {

class AsioTimer
{
public:
    explicit AsioTimer(const function<void ()> &cb);
    virtual ~AsioTimer();
    AsioTimer( const AsioTimer & ) = delete;
    const AsioTimer & operator = ( const AsioTimer& ) = delete;

    void startRelative(const pnxs::Duration & duration);
    void startAbsolute(const pnxs::SteadyTimePoint& timepoint);

private:

    using timer_t = boost::asio::steady_timer;
    using duration_t = timer_t::clock_type::duration;

    void onTimeout(const boost::system::error_code& error);

    timer_t m_timer;
    function<void ()> m_cb;
};

class AsioSingleShotTimer : public AsioTimer
{
    const function<void ()> m_cb;
    unsigned int m_id;
    pnxs::Duration m_interval;
    pnxs::SteadyTimePoint m_next;
    bool m_periodic;

	inline static unsigned int m_lastTimerId = 1;

    void callCb();

public:
    AsioSingleShotTimer(const pnxs::Duration & interval, const function<void ()> &cb, bool periodic = false);
    ~AsioSingleShotTimer() override;

    inline static std::map<unsigned int, AsioSingleShotTimer*> s_all;

    unsigned int id() { return m_id; }
    static void remTimer(unsigned int id);
};


}
