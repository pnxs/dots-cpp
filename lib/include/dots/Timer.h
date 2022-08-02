// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <memory>
#include <dots/tools/Handler.h>
#include <dots/type/Chrono.h>
#include <dots/asio_forward.h>

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
    struct [[nodiscard]] Timer
    {
        // DEPRECATED: only required for backwards compatibility
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
        Timer(Timer&& other) = default;

        /*!
         * @brief Destroy the Timer object.
         *
         * When the Timer object is destroyed and is managing an active timer
         * (i.e. the timer did not yet run out or is periodic), the timer will
         * safely be cancelled without invoking the handler given in Timer().
         */
        ~Timer();

        Timer& operator = (const Timer& rhs) = delete;
        Timer& operator = (Timer&& rhs) = default;

        /*!
         * @brief Release management of the timer.
         *
         * Calling this function will decouple the timer from the Timer
         * object's lifetime, without invoking the timeout handler. As a
         * result, this Timer object will be empty when the function returns.
         *
         * Note that this will have no effect if the Timer object is already
         * empty when the function is called.
         *
         * @warning Calling this function will make it impossible to manually
         * cancel the timer.
         *
         * @remark This function is intended to cover simple use cases where
         * the durations of timers are bound to an application's lifetime and
         * management is not required.
         */
        void discard();

    private:

        struct timer_data;

        static void StartRelative(const std::shared_ptr<timer_data>& timerData);
        static void StartAbsolute(const std::shared_ptr<timer_data>& timerData);
        static void AsyncWait(const std::shared_ptr<timer_data>& timerData);

        std::shared_ptr<timer_data> m_timerData;
    };
}
