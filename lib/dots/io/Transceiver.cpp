#include <dots/io/Transceiver.h>
#include <dots/common/logging.h>
#include <dots/io/serialization/AsciiSerialization.h>

namespace dots
{
	const io::ChannelConnection& Transceiver::openConnection(io::ChannelConnection connection)
	{
		if (m_connection != std::nullopt)
        {
            throw std::logic_error{ "already connected" };
        }
		
		m_connection.emplace(std::move(connection));
		m_connection->asyncReceive(m_registry,
			[this](const DotsHeader& header, Transmission&& transmission){ return handleReceive(header, std::move(transmission)); },
			[this](const std::exception& e){ handleError(e); }
		);
		
		return *m_connection;
	}

	void Transceiver::closeConnection()
	{
		m_connection != std::nullopt;
	}

	const io::Registry& Transceiver::registry() const
	{
		return m_registry;
	}

	io::Registry& Transceiver::registry()
	{
		return m_registry;
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

		m_connection->joinGroup(descriptor.name());
		return m_dispatcher.subscribe(descriptor, std::move(handler));
	}

	Subscription Transceiver::subscribe(const type::StructDescriptor<>& descriptor, event_handler_t<>&& handler)
	{
		if (descriptor.substructOnly())
		{
			throw std::logic_error{ "attempt to subscribe to substruct-only type: " + descriptor.name() };
		}

		m_connection->joinGroup(descriptor.name());
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

	void Transceiver::publish(const type::Struct& instance, types::property_set_t includedProperties/*t = types::property_set_t::All*/, bool remove/* = false*/)
	{
		const type::StructDescriptor<>& descriptor = instance._descriptor();
		
		if (descriptor.substructOnly())
		{
			throw std::logic_error{ "attempt to publish substruct-only type: " + descriptor.name() };
		}		

    	if (!(descriptor.keyProperties() <= includedProperties))
	    {
	        throw std::runtime_error("tried to publish instance with invalid key (not all key-fields are set) what=" + includedProperties.toString() + " tdkeys=" + descriptor.keyProperties().toString());
	    }
		
		LOG_DATA_S("data:" << to_ascii(&descriptor, &instance, includedProperties));
		m_connection->transmit(instance, includedProperties, remove);
		
	}

	void Transceiver::remove(const type::Struct& instance)
	{
		publish(instance, instance._keyProperties(), true);
	}

	void Transceiver::publish(const type::StructDescriptor<>*/* td*/, const type::Struct& instance, types::property_set_t what, bool remove)
	{
		publish(instance, what, remove);
	}

	bool Transceiver::handleReceive(const DotsHeader& header, Transmission&& transmission)
	{
		try 
        {
            m_dispatcher.dispatch(header, transmission.instance());
            return true;
        }
        catch (const std::exception& e) 
        {
            handleError(e);
            return false;
        }
	}
	
	void Transceiver::handleError(const std::exception& e)
	{
		LOG_ERROR_S("connection error in async receive: " << e.what());
		closeConnection();
	}
}