// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#undef DOTS_NO_GLOBAL_TRANSCEIVER
#include <optional>
#include <dots/dots.h>
#include <dots/io/Io.h>

namespace dots
{
    namespace details
    {
        struct TimerService : asio::execution_context::service
        {
            using key_type = TimerService;
            using callback_t = tools::Handler<void()>;

            explicit TimerService(asio::execution_context& executionContext) :
                asio::execution_context::service(executionContext),
                m_lastTimerId(0)
            {
                /* do nothing */
            }
            TimerService(const TimerService& other) = delete;
            TimerService(TimerService&& other) noexcept(false) = delete;
            ~TimerService() = default;

            TimerService& operator = (const TimerService& rhs) = delete;
            TimerService& operator = (TimerService&& rhs) noexcept(false) = delete;

            Timer::id_t addTimer(type::Duration timeout, callback_t cb, bool periodic)
            {
                Timer::id_t id = ++m_lastTimerId;
                m_timers.try_emplace(id, static_cast<asio::io_context&>(context()), timeout, std::move(cb), periodic);

                return id;
            }

            void removeTimer(unsigned id)
            {
                m_timers.erase(id);
            }

        private:

            void shutdown() noexcept override
            {
                m_timers.clear();
            }

            Timer::id_t m_lastTimerId;
            std::map<Timer::id_t, Timer> m_timers;
        };
    }

    static std::optional<GuestTransceiver> g_global_transceiver;
    static std::shared_ptr<GuestTransceiver> g_global_transceiver_ptr;

    Timer::id_t add_timer(type::Duration timeout, tools::Handler<void()> handler, bool periodic/* = false*/)
    {
        return io::global_service<details::TimerService>().addTimer(timeout, std::move(handler), periodic);
    }

    void remove_timer(Timer::id_t id)
    {
        io::global_service<details::TimerService>().removeTimer(id);
    }

    Timer create_timer(type::Duration timeout, tools::Handler<void()> handler, bool periodic)
    {
        return Timer{ io::global_io_context(), timeout, std::move(handler), periodic };
    }

    GuestTransceiver& global_transceiver()
    {
        assert(g_global_transceiver.has_value());
        return g_global_transceiver.value();
    }

    bool global_transceiver_is_set()
    {
        return g_global_transceiver.has_value();
    }

    GuestTransceiver& global_transceiver_create(GuestTransceiver&& transceiver)
    {
        if (g_global_transceiver.has_value()) {
            throw std::runtime_error("global_transceiver is already set.");
        }
        g_global_transceiver = std::move(transceiver);
        return global_transceiver();
    }

    void global_transceiver_destroy()
    {
        if (!g_global_transceiver.has_value()) {
            throw std::runtime_error("global_transceiver_destroy: no transceiver instantiated.");
        }
        global_transceiver().clear();
        g_global_transceiver.reset();
    }

    void publish(const type::Struct& instance, std::optional<property_set_t> includedProperties/* = std::nullopt*/, bool remove/* = false*/)
    {
        global_transceiver().publish(instance, includedProperties, remove);
    }

    void remove(const type::Struct& instance)
    {
        publish(instance, instance._keyProperties(), true);
    }

    Subscription subscribe(const type::StructDescriptor& descriptor, Transceiver::event_handler_t<> handler)
    {
        return global_transceiver().subscribe(descriptor, std::move(handler));
    }

    const ContainerPool& pool()
    {
        return global_transceiver().pool();
    }

    const Container<>& container(const type::StructDescriptor& descriptor)
    {
        return global_transceiver().container(descriptor);
    }
}
