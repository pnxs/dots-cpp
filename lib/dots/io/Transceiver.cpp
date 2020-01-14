#include <dots/io/Transceiver.h>
#include <dots/common/logging.h>
#include <dots/io/serialization/AsciiSerialization.h>

namespace dots
{
	const io::Connection& Transceiver::open(const std::string_view& selfName, channel_ptr_t channel, bool server, io::Connection::descriptor_map_t preloadPublishTypes/* = {}*/, io::Connection::descriptor_map_t preloadSubscribeTypes/* = {}*/)
	{
		if (m_connection != std::nullopt)
        {
            throw std::logic_error{ "already connected" };
        }
		
		m_connection.emplace(std::move(channel), server, std::move(preloadPublishTypes), std::move(preloadSubscribeTypes));
		m_connection->asyncReceive(m_registry, selfName,
			[this](io::Connection& connection, const DotsTransportHeader& header, Transmission&& transmission, bool isFromMyself){ return handleReceive(connection, header, std::move(transmission), isFromMyself); },
			[this](io::Connection& connection, const std::exception* e){ handleClose(connection, e); }
		);
		
		return *m_connection;
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

	bool Transceiver::handleReceive(io::Connection& connection, const DotsTransportHeader& header, Transmission&& transmission, bool isFromMyself)
	{
		try 
        {
            m_dispatcher.dispatch(header.dotsHeader, transmission.instance(), isFromMyself);
            return true;
        }
        catch (const std::exception& e) 
        {
            handleClose(connection, &e);
            return false;
        }
	}
	
	void Transceiver::handleClose(io::Connection& connection, const std::exception* e)
	{
		if (e != nullptr)
		{
		    LOG_ERROR_S("connection error: " << e->what());
		    
		}

		m_connection = std::nullopt;
		LOG_INFO_S("connection closed -> peerId: " << connection.peerId() << ", name: " << connection.peerName());
	}
}