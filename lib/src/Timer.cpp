#include <dots/Timer.h>
#include <dots/io/services/TimerService.h>

namespace dots
{
    using timer_t = asio::steady_timer;
    using duration_t = timer_t::clock_type::duration;

    struct Timer::timer_data
    {
        timer_t timer;
        handler_t handler;
        type::Duration interval;
        type::SteadyTimePoint next;
        bool periodic;
    };

    Timer::Timer(asio::io_context& ioContext, type::Duration interval, handler_t handler, bool periodic) :
        m_timerData{ std::make_shared<timer_data>(timer_data{
            timer_t{ ioContext },
            std::move(handler),
            interval,
            type::SteadyTimePoint::Now(),
            periodic
        } ) }
    {
        if (m_timerData->periodic)
        {
            startAbsolute(m_timerData->next += m_timerData->interval);
        }
        else
        {
            startRelative(m_timerData->interval);
        }
    }

    Timer::~Timer()
    {
        try
        {
            m_timerData->timer.cancel();
        }
        catch (...)
        {
            /* do nothing */
        }
    }

    void Timer::startRelative(type::Duration duration)
    {
        m_timerData->timer.expires_after(std::chrono::duration_cast<duration_t>(duration));
        asyncWait();
    }

    void Timer::startAbsolute(type::SteadyTimePoint timepoint)
    {
        m_timerData->timer.expires_at(std::chrono::time_point_cast<duration_t>(timepoint));
        asyncWait();
    }

    void Timer::asyncWait()
    {
        m_timerData->timer.async_wait([this, timerData{ m_timerData }](boost::system::error_code error)
        {
            if (timerData.use_count() == 1 || error == asio::error::operation_aborted)
            {
                return;
            }

            m_timerData->handler();

            if (m_timerData->periodic)
            {
                startAbsolute(m_timerData->next += m_timerData->interval);
            }
        });
    }
}
