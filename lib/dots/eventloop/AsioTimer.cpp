#include "AsioTimer.h"
#include "IoService.h"
#include "dots/functional/fun.h"

namespace dots
{

AsioTimer::AsioTimer(const function<void()> &cb)
:m_timer(ioService())
,m_cb(cb)
{

}

AsioTimer::~AsioTimer()
{
    m_timer.cancel();
}

void AsioTimer::onTimeout(const boost::system::error_code &error)
{
    if (error != boost::asio::error::operation_aborted)
    {
        m_cb();
    }
}


void AsioTimer::startRelative(const pnxs::Duration &duration)
{
    m_timer.expires_from_now(std::chrono::duration_cast<duration_t>(duration));
    m_timer.async_wait(FUN(*this, onTimeout));
}

void AsioTimer::startAbsolute(const pnxs::SteadyTimePoint& timepoint)
{
    m_timer.expires_at(std::chrono::time_point_cast<duration_t>(timepoint));
    m_timer.async_wait(FUN(*this, onTimeout));
}

AsioSingleShotTimer::AsioSingleShotTimer(const pnxs::Duration &interval, const function<void()> &cb, bool periodic)
:AsioTimer(std::bind(&AsioSingleShotTimer::callCb, this))
,m_cb(cb)
,m_id(m_lastTimerId++)
,m_interval(interval)
,m_next(pnxs::SteadyNow{})
,m_periodic(periodic)
{
    s_all[m_id] = this;

    if (m_periodic)
    {
        startAbsolute(m_next += m_interval);
    }
    else
    {
        startRelative(m_interval);
    }
}

AsioSingleShotTimer::~AsioSingleShotTimer()
{
    s_all.erase(m_id);
}

void AsioSingleShotTimer::remTimer(unsigned int id)
{
    auto iter = s_all.find(id);
    if (iter != s_all.end())
    {
        delete iter->second;
    }
}

void AsioSingleShotTimer::callCb()
{
    if (m_periodic)
    {
        m_cb();
        startAbsolute(m_next += m_interval);
    }
    else
    {
        m_cb();
        remTimer(m_id);
    }
}

}
