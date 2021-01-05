#include <dots/io/Timer.h>
#include <dots/tools/fun.h>
#include <dots/io/services/TimerService.h>

namespace dots::io
{
    Timer::Timer(boost::asio::io_context& ioContext, id_t id, const type::Duration& interval, const callback_t& cb, bool periodic) :
        m_this{ std::make_shared<Timer*>(this) },
        m_timer(ioContext),
        m_cb(cb),
        m_id(id),
        m_interval(interval),
        m_next(type::SteadyTimePoint::Now()),
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

    Timer::~Timer()
    {
        m_timer.cancel();
    }

    void Timer::startRelative(const type::Duration & duration)
    {
        m_timer.expires_after(std::chrono::duration_cast<duration_t>(duration));
        asyncWait();
    }

    void Timer::startAbsolute(const type::SteadyTimePoint & timepoint)
    {
        m_timer.expires_at(std::chrono::time_point_cast<duration_t>(timepoint));
        asyncWait();
    }

    void Timer::asyncWait()
    {
        m_timer.async_wait([this, this_{ std::weak_ptr<Timer*>(m_this) }](boost::system::error_code error)
        {
            if (this_.expired() || error == boost::asio::error::operation_aborted)
            {
                return;
            }

            m_cb();

            if (m_periodic)
            {
                startAbsolute(m_next += m_interval);
            }
            else
            {
                boost::asio::use_service<TimerService>(static_cast<boost::asio::execution_context&>(m_timer.get_executor().context())).removeTimer(m_id);
            }
        });
    }
}