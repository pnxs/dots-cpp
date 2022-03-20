#pragma once

#include <dots/tools/Handler.h>
#include <dots/type/Chrono.h>
#include <dots/asio.h>

namespace dots
{
    /*!
     * @class Timer Timer.h <dots/Timer.h>
     *
     * @brief Scoped resource for active DOTS timers.
     *
     * An object of this class is a RAII-style resource that represents an
     * active DOTS timer.
     *
     * The timer will run asynchronously in a given IO context and invoke a
     * given handler after it runs out.
     *
     * Note that Timer objects can safely be destroyed prematurely, in
     * which case the timer will be cancelled without invoking the handler.
     */
    struct Timer
    {
        using id_t = uint32_t;
        using handler_t = tools::Handler<void()>;

        /*!
         * @brief Construct a new Timer object.
         *
         * This will create and start a timer within a given IO context.
         *
         * @param ioContext The IO context (i.e. event loop) to associate with
         * the timer.
         *
         * @param interval The duration of the timer. The handler \p cb will be
         * invoked when this amount of time has passed.
         *
         * @param handler The handler to invoke asynchronously after the timer runs
         * out.
         *
         * @param periodic Specifies whether the timer will be restarted after
         * it ran out and @p cb was invoked.
         */
        Timer(asio::io_context& ioContext, type::Duration interval, handler_t handler, bool periodic = false);
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

    private:

        struct timer_data;

        static void StartRelative(const std::shared_ptr<timer_data>& timerData);
        static void StartAbsolute(const std::shared_ptr<timer_data>& timerData);
        static void AsyncWait(const std::shared_ptr<timer_data>& timerData);

        std::shared_ptr<timer_data> m_timerData;
    };
}
