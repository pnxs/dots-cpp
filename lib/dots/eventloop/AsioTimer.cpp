#include "AsioTimer.h"
#include "IoService.h"
#include "dots/functional/fun.h"

namespace dots
{

std::map<unsigned int, AsioSingleShotTimer*> AsioSingleShotTimer::s_all;
unsigned int AsioSingleShotTimer::m_lastTimerId = 1;

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


void AsioTimer::start(const pnxs::Duration &interval)
{
    m_timer.expires_from_now(std::chrono::milliseconds(interval.toMs()));
    m_timer.async_wait(FUN(*this, onTimeout));
}

unsigned int AsioTimer::singleShot(const pnxs::Duration &interval, const function<void()> &cb)
{
    auto timer = new AsioSingleShotTimer(interval, cb);
    return timer->id();
}



AsioSingleShotTimer::AsioSingleShotTimer(const pnxs::Duration &interval, const function<void()> &cb)
:AsioTimer(std::bind(&AsioSingleShotTimer::callCb, this))
,m_cb(cb)
,m_id(m_lastTimerId++)
{
    s_all[m_id] = this;
    start(interval);
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
    //printf("t\n");
    m_cb();
    remTimer(m_id);
}

}
