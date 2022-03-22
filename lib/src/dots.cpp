#undef DOTS_NO_GLOBAL_TRANSCEIVER
#include <optional>
#include <dots/dots.h>
#include <dots/io/Io.h>
#define DOTS_ACKNOWLEDGE_DEPRECATION_OF_TimerService
#include <dots/io/services/TimerService.h>

namespace dots
{
    Timer::id_t add_timer(type::Duration timeout, tools::Handler<void()> handler, bool periodic/* = false*/)
    {
        return io::global_service<io::TimerService>().addTimer(timeout, std::move(handler), periodic);
    }

    void remove_timer(Timer::id_t id)
    {
        io::global_service<io::TimerService>().removeTimer(id);
    }

    Timer create_timer(type::Duration timeout, tools::Handler<void()> handler, bool periodic)
    {
        return Timer{ io::global_io_context(), timeout, std::move(handler), periodic };
    }

    std::optional<GuestTransceiver>& global_transceiver()
    {
        static std::optional<GuestTransceiver> Transceiver;
        return Transceiver;
    }

    void publish(const type::Struct& instance, std::optional<property_set_t> includedProperties/* = std::nullopt*/, bool remove/* = false*/)
    {
        global_transceiver()->publish(instance, includedProperties, remove);
    }

    void remove(const type::Struct& instance)
    {
        publish(instance, instance._keyProperties(), true);
    }

    Subscription subscribe(const type::StructDescriptor<>& descriptor, Transceiver::event_handler_t<> handler)
    {
        return global_transceiver()->subscribe(descriptor, std::move(handler));
    }

    const ContainerPool& pool()
    {
        return global_transceiver()->pool();
    }

    const Container<>& container(const type::StructDescriptor<>& descriptor)
    {
        return global_transceiver()->container(descriptor);
    }
}
