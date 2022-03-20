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
            StartAbsolute(m_timerData);
        }
        else
        {
            StartRelative(m_timerData);
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

    void Timer::StartRelative(const std::shared_ptr<timer_data>& timerData)
    {
        type::Duration duration = timerData->interval;
        timerData->timer.expires_after(std::chrono::duration_cast<duration_t>(duration));
        AsyncWait(timerData);
    }

    void Timer::StartAbsolute(const std::shared_ptr<timer_data>& timerData)
    {
        type::SteadyTimePoint timepoint = timerData->next += timerData->interval;
        timerData->timer.expires_at(std::chrono::time_point_cast<duration_t>(timepoint));
        AsyncWait(timerData);
    }

    void Timer::AsyncWait(const std::shared_ptr<timer_data>& timerData)
    {
        timerData->timer.async_wait([timerData](boost::system::error_code error)
        {
            if (timerData.use_count() == 1 || error == asio::error::operation_aborted)
            {
                return;
            }

            timerData->handler();

            if (timerData->periodic)
            {
                StartAbsolute(timerData);
            }
        });
    }
}
