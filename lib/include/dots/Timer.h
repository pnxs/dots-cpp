#pragma once

#include <dots/type/Chrono.h>
#include <boost/asio.hpp>

namespace dots
{
    /*!
     * @class Timer Timer.h <dots/Timer.h>
     *
     * @brief Scoped resource for active DOTS timers.
     *
     * An object of this class is a RAII-style resource that represents an
     * active DOTS timer (e.g. created by dots::add_timer()).
     *
     * The timer will run asynchronously in a given IO context and invoke a
     * given handler after it runs out.
     *
     * Note that Timer objects can safely be destroyed prematurely, in
     * which case the timer will be cancelled without invoking the handler.
     *
     * @attention Outside of advanced use cases, a regular user is never
     * required to create Timer objects themselves. Instead, they are
     * always obtained by using the dots::io::TimerService.
     */
    struct Timer
    {
        using id_t = uint32_t;
        using callback_t = std::function<void()>;

        /*!
         * @brief Construct a new Timer object.
         *
         * This will create and start a timer within a given IO context.
         *
         * @param ioContext The IO context (i.e. event loop) to associate with
         * the timer.
         *
         * @param id A (unique) id to identify the timer.
         *
         * @param interval The duration of the timer. The handler \p cb will be
         * invoked when this amount of time has passed.
         *
         * @param cb The handler to invoke asynchronously after the timer runs
         * out.
         *
         * @param periodic Specifies whether the timer will be restarted after
         * it ran out and @p cb was invoked.
         */
        Timer(boost::asio::io_context& ioContext, id_t id, type::Duration interval, callback_t cb, bool periodic = false);
        Timer(const Timer& other) = delete;
        Timer(Timer&& other) = delete;

        /*!
         * @brief Destroy the Timer object.
         *
         * When the Timer object is destroyed and is managing an active timer
         * (i.e. the timer did not yet run out or is periodic), the timer will
         * safely be cancelled without invoking the handler given in Timer().
         */
        ~Timer();

        Timer& operator = (const Timer& rhs) = delete;
        Timer& operator = (Timer&& rhs) = delete;

        /*!
         * @brief Get the id of the timer.
         *
         * Note that this is the same id given in Timer().
         *
         * @return id_t The id of the timer.
         */
        id_t id() const { return m_id; }

    private:

        using timer_t = boost::asio::steady_timer;
        using duration_t = timer_t::clock_type::duration;

        void startRelative(type::Duration duration);
        void startAbsolute(type::SteadyTimePoint timepoint);
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