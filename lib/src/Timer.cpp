#include <dots/Timer.h>
#include <dots/tools/fun.h>
#include <dots/io/services/TimerService.h>

namespace dots
{
    Timer::Timer(boost::asio::io_context& ioContext, id_t id, type::Duration interval, callback_t cb, bool periodic) :
        m_this{ std::make_shared<Timer*>(this) },
        m_timer(ioContext),
        m_cb(std::move(cb)),
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
        try
        {
            m_timer.cancel();
        }
        catch (...)
        {
            /* do nothing */
        }
    }

    void Timer::startRelative(type::Duration duration)
    {
        m_timer.expires_after(std::chrono::duration_cast<duration_t>(duration));
        asyncWait();
    }

    void Timer::startAbsolute(type::SteadyTimePoint timepoint)
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
                boost::asio::use_service<io::TimerService>(static_cast<boost::asio::execution_context&>(m_timer.get_executor().context())).removeTimer(m_id);
            }
        });
    }
}