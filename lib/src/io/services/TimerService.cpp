#include <dots/io/services/TimerService.h>

namespace dots::io
{
    TimerService::TimerService(asio::execution_context& executionContext) :
        asio::execution_context::service(executionContext),
        m_lastTimerId(0)
    {
        /* do nothing */
    }

    auto TimerService::addTimer(type::Duration timeout, callback_t cb, bool periodic) -> Timer::id_t
    {
        Timer::id_t id = ++m_lastTimerId;
        m_timers.try_emplace(id, static_cast<asio::io_context&>(context()), id, timeout, std::move(cb), periodic);

        return id;
    }

    void TimerService::removeTimer(unsigned id)
    {
        m_timers.erase(id);
    }

    void TimerService::shutdown() noexcept
    {
        m_timers.clear();
    }
}
