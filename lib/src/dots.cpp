#undef DOTS_NO_GLOBAL_TRANSCEIVER
#include <optional>
#include <dots/dots.h>
#include <dots/io/Io.h>
#include <dots/io/services/TimerService.h>
#if defined(BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR)
#include <dots/io/services/FdHandlerService.h>
#endif

namespace dots
{
    inline std::optional<GuestTransceiver> GlobalTransceiver;

    Timer::id_t add_timer(const type::Duration& timeout, const std::function<void()>& handler, bool periodic/* = false*/)
    {
        return io::global_service<io::TimerService>().addTimer(timeout, handler, periodic);
    }

    void remove_timer(Timer::id_t id)
    {
        io::global_service<io::TimerService>().removeTimer(id);
    }

    #if defined(BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR)
    void add_fd_handler(int fileDescriptor, const std::function<void()>& handler)
    {
        io::global_service<io::FdHandlerService>().addInEventHandler(fileDescriptor, handler);
    }

    void remove_fd_handler(int fileDescriptor)
    {
        io::global_service<io::FdHandlerService>().removeInEventHandler(fileDescriptor);
    }
    #endif

    GuestTransceiver& transceiver(const std::string_view& name/* = "dots-transceiver"*/, bool reset/* = false*/)
    {
        if (GlobalTransceiver == std::nullopt || reset)
        {
            GlobalTransceiver.emplace(std::string{ name });
        }

        return *GlobalTransceiver;
    }

    void publish(const type::Struct& instance, std::optional<types::property_set_t> includedProperties/* = std::nullopt*/, bool remove/* = false*/)
    {
        transceiver().publish(instance, includedProperties == std::nullopt ? instance._validProperties() : *includedProperties, remove);
    }

    void remove(const type::Struct& instance)
    {
        publish(instance, instance._keyProperties(), true);
    }

    Subscription subscribe(const type::StructDescriptor<>& descriptor, Transceiver::transmission_handler_t&& handler)
    {
        return transceiver().subscribe(descriptor, std::move(handler));
    }

    Subscription subscribe(const type::StructDescriptor<>& descriptor, Transceiver::event_handler_t<>&& handler)
    {
        return transceiver().subscribe(descriptor, std::move(handler));
    }

    const ContainerPool& pool()
    {
        return transceiver().pool();
    }

    const Container<>& container(const type::StructDescriptor<>& descriptor)
    {
        return transceiver().container(descriptor);
    }
}
