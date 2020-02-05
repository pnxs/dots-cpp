#include <dots/io/Transceiver.h>
#include <dots/common/logging.h>
#include <dots/io/serialization/AsciiSerialization.h>
#include <DotsMember.dots.h>

namespace dots
{
    Transceiver::Transceiver(std::string selfName) :
	    m_registry{ [&](const type::Descriptor<>& descriptor){ handleNewType(descriptor); } },
        m_selfName{ std::move(selfName) }
    {
		/* do nothing */
    }

	const std::string& Transceiver::selfName() const
    {
        return m_selfName;
    }

	const io::Registry& Transceiver::registry() const
	{
		return m_registry;
	}

	io::Registry& Transceiver::registry()
	{
		return m_registry;
	}

    Dispatcher& Transceiver::dispatcher()
    {
		return m_dispatcher;
    }

    const ContainerPool& Transceiver::pool() const
	{
		return m_dispatcher.pool();
	}

	const Container<>& Transceiver::container(const type::StructDescriptor<>& descriptor)
	{
		return m_dispatcher.container(descriptor);
	}

	Subscription Transceiver::subscribe(const type::StructDescriptor<>& descriptor, receive_handler_t<>&& handler)
	{
		if (descriptor.substructOnly())
		{
			throw std::logic_error{ "attempt to subscribe to substruct-only type: " + descriptor.name() };
		}

		joinGroup(descriptor.name());
		return m_dispatcher.subscribe(descriptor, std::move(handler));
	}

	Subscription Transceiver::subscribe(const type::StructDescriptor<>& descriptor, event_handler_t<>&& handler)
	{
		if (descriptor.substructOnly())
		{
			throw std::logic_error{ "attempt to subscribe to substruct-only type: " + descriptor.name() };
		}

		joinGroup(descriptor.name());
		return m_dispatcher.subscribe(descriptor, std::move(handler));
	}

	Subscription Transceiver::subscribe(const std::string_view& name, receive_handler_t<>&& handler)
	{
		return subscribe(m_registry.getStructType(name), std::move(handler));
	}

	Subscription Transceiver::subscribe(const std::string_view& name, event_handler_t<>&& handler)
	{
		return subscribe(m_registry.getStructType(name), std::move(handler));
	}

	void Transceiver::subscribe(new_type_handler_t&& handler)
    {
		const new_type_handler_t& handler_ = m_newTypeHandlers.emplace_back(std::move(handler));

		for (const auto& [name, descriptor] : type::StaticDescriptorMap::Descriptors())
        {
			(void)name;
			handler_(*descriptor);
        }

		for (const auto& [name, descriptor] : m_registry)
		{
		    (void)name;
            handler_(*descriptor);
		}
    }

    void Transceiver::remove(const type::Struct& instance)
	{
		publish(instance, instance._keyProperties(), true);
	}

	void Transceiver::publish(const type::StructDescriptor<>*/* td*/, const type::Struct& instance, types::property_set_t what, bool remove)
	{
		publish(instance, what, remove);
	}

	void Transceiver::handleNewType(const type::Descriptor<>& descriptor) noexcept
    {
		for (const new_type_handler_t& handler : m_newTypeHandlers)
		{
            try
            {
				handler(descriptor);
            }
            catch (const std::exception& e)
            {
				LOG_ERROR_S("error in new type handler -> " << e.what());
            }
		}
    }
}