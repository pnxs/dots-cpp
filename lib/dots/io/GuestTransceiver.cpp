#include <dots/io/GuestTransceiver.h>
#include <dots/common/logging.h>
#include <dots/io/serialization/AsciiSerialization.h>
#include <DotsMember.dots.h>

namespace dots
{
    GuestTransceiver::GuestTransceiver(std::string selfName) :
	    m_name{ std::move(selfName) }
    {
		/* do nothing */
    }

	const io::Connection& GuestTransceiver::open(channel_ptr_t channel, bool server, descriptor_map_t preloadPublishTypes/* = {}*/, descriptor_map_t preloadSubscribeTypes/* = {}*/)
	{
		if (m_connection != std::nullopt)
        {
            throw std::logic_error{ "already connected" };
        }

		m_preloadPublishTypes = std::move(preloadPublishTypes);
		m_preloadSubscribeTypes = std::move(preloadSubscribeTypes);
		
		m_connection.emplace(std::move(channel), server);
		m_connection->asyncReceive(m_registry, m_name,
			[this](io::Connection& connection, const DotsTransportHeader& header, Transmission&& transmission, bool isFromMyself){ return handleReceive(connection, header, std::move(transmission), isFromMyself); },
			[this](io::Connection& connection, const std::exception* e){ handleTransition(connection, e); }
		);
		
		return *m_connection;
	}

	const io::Registry& GuestTransceiver::registry() const
	{
		return m_registry;
	}

	io::Registry& GuestTransceiver::registry()
	{
		return m_registry;
	}

	const ContainerPool& GuestTransceiver::pool() const
	{
		return m_dispatcher.pool();
	}

	const Container<>& GuestTransceiver::container(const type::StructDescriptor<>& descriptor)
	{
		return m_dispatcher.container(descriptor);
	}

	Subscription GuestTransceiver::subscribe(const type::StructDescriptor<>& descriptor, receive_handler_t<>&& handler)
	{
		if (descriptor.substructOnly())
		{
			throw std::logic_error{ "attempt to subscribe to substruct-only type: " + descriptor.name() };
		}

		joinGroup(descriptor.name());
		return m_dispatcher.subscribe(descriptor, std::move(handler));
	}

	Subscription GuestTransceiver::subscribe(const type::StructDescriptor<>& descriptor, event_handler_t<>&& handler)
	{
		if (descriptor.substructOnly())
		{
			throw std::logic_error{ "attempt to subscribe to substruct-only type: " + descriptor.name() };
		}

		joinGroup(descriptor.name());
		return m_dispatcher.subscribe(descriptor, std::move(handler));
	}

	Subscription GuestTransceiver::subscribe(const std::string_view& name, receive_handler_t<>&& handler)
	{
		return subscribe(m_registry.getStructType(name), std::move(handler));
	}

	Subscription GuestTransceiver::subscribe(const std::string_view& name, event_handler_t<>&& handler)
	{
		return subscribe(m_registry.getStructType(name), std::move(handler));
	}

	void GuestTransceiver::publish(const type::Struct& instance, types::property_set_t includedProperties/*t = types::property_set_t::All*/, bool remove/* = false*/)
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

	void GuestTransceiver::remove(const type::Struct& instance)
	{
		publish(instance, instance._keyProperties(), true);
	}

	void GuestTransceiver::publish(const type::StructDescriptor<>*/* td*/, const type::Struct& instance, types::property_set_t what, bool remove)
	{
		publish(instance, what, remove);
	}

	void GuestTransceiver::joinGroup(const std::string_view& name)
	{
		LOG_DEBUG_S("send DotsMember (join " << name << ")");
		m_connection->transmit(DotsMember{
            DotsMember::groupName_i{ name },
            DotsMember::event_i{ DotsMemberEvent::join }
        });
	}

	void GuestTransceiver::leaveGroup(const std::string_view& name)
	{
		LOG_DEBUG_S("send DotsMember (leave " << name << ")");
		m_connection->transmit(DotsMember{
            DotsMember::groupName_i{ name },
            DotsMember::event_i{ DotsMemberEvent::leave }
        });
	}

	bool GuestTransceiver::handleReceive(io::Connection& connection, const DotsTransportHeader& header, Transmission&& transmission, bool isFromMyself)
	{
		try 
        {
            m_dispatcher.dispatch(header.dotsHeader, transmission.instance(), isFromMyself);
            return true;
        }
        catch (const std::exception& e) 
        {
            handleTransition(connection, &e);
            return false;
        }
	}
	
	void GuestTransceiver::handleTransition(io::Connection& connection, const std::exception* e)
	{
		if (connection.state() == DotsConnectionState::early_subscribe)
		{
		    for (const auto& [name, descriptor] : m_preloadPublishTypes)
			{
				(void)name;
				connection.transmit(*descriptor);
			}

			for (const auto& [name, descriptor] : m_preloadSubscribeTypes)
			{
				connection.transmit(*descriptor);			
				joinGroup(name);
			}

			m_preloadPublishTypes.clear();
			m_preloadSubscribeTypes.clear();
		}
		else if (connection.state() == DotsConnectionState::closed)
		{
		    if (e != nullptr)
		    {
		        LOG_ERROR_S("connection error: " << e->what());
		        
		    }

		    m_connection = std::nullopt;
		    LOG_INFO_S("connection closed -> peerId: " << connection.peerId() << ", name: " << connection.peerName());
		}
	}
}