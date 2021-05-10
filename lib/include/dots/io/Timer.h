#pragma once

#include <dots/type/Chrono.h>
#include <boost/asio.hpp>

namespace dots::io
{
    struct Timer
    {
        using id_t = uint32_t;
        using callback_t = std::function<void()>;

        Timer(boost::asio::io_context& ioContext, id_t id, const type::Duration& interval, const callback_t& cb, bool periodic = false);
        Timer(const Timer& other) = delete;
        Timer(Timer&& other) = delete;
        ~Timer();

        Timer& operator = (const Timer& rhs) = delete;
        Timer& operator = (Timer&& rhs) = delete;

        id_t id() { return m_id; }

        type::Duration interval() const { return m_interval; }

    private:

        using timer_t = boost::asio::steady_timer;
        using duration_t = timer_t::clock_type::duration;

        void startRelative(const type::Duration& duration);
        void startAbsolute(const type::SteadyTimePoint& timepoint);
        void asyncWait();

        std::shared_ptr<Timer*> m_this;
        timer_t m_timer;
        callback_t m_cb;
        id_t m_id;
        type::Duration m_interval;
        type::SteadyTimePoint m_next;
        bool m_periodic;
    };
}
