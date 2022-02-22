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

    Timer::id_t add_timer(type::Duration timeout, tools::Handler<void()> handler, bool periodic/* = false*/)
    {
        return io::global_service<io::TimerService>().addTimer(timeout, std::move(handler), periodic);
    }

    void remove_timer(Timer::id_t id)
    {
        io::global_service<io::TimerService>().removeTimer(id);
    }

    #if defined(BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR)
    void add_fd_handler(int fileDescriptor, tools::Handler<void()> handler)
    {
        io::global_service<io::posix::FdHandlerService>().addInEventHandler(fileDescriptor, std::move(handler));
    }

    void remove_fd_handler(int fileDescriptor)
    {
        io::global_service<io::posix::FdHandlerService>().removeInEventHandler(fileDescriptor);
    }
    #endif

    GuestTransceiver& set_transceiver(std::string_view name/* = "dots-transceiver"*/, std::optional<GuestTransceiver::transition_handler_t> transitionHandler/* = std::nullopt*/)
    {
        return GlobalTransceiver.emplace(std::string{ name }, io::global_io_context(), type::Registry::StaticTypePolicy::All, std::move(transitionHandler));
    }

    GuestTransceiver& transceiver()
    {
        if (GlobalTransceiver == std::nullopt)
        {
            return set_transceiver();
        }
        else
        {
            return *GlobalTransceiver;
        }
    }

    void publish(const type::Struct& instance, std::optional<property_set_t> includedProperties/* = std::nullopt*/, bool remove/* = false*/)
    {
        transceiver().publish(instance, includedProperties, remove);
    }

    void remove(const type::Struct& instance)
    {
        publish(instance, instance._keyProperties(), true);
    }

    Subscription subscribe(const type::StructDescriptor<>& descriptor, Transceiver::event_handler_t<> handler)
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
