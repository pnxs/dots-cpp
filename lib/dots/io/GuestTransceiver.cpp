#include <dots/io/GuestTransceiver.h>
#include <dots/common/logging.h>
#include <dots/io/serialization/AsciiSerialization.h>
#include <DotsMember.dots.h>

namespace dots
{
    GuestTransceiver::GuestTransceiver(std::string selfName) :
	    Transceiver(std::move(selfName))
    {
		/* do nothing */
    }

	const io::Connection& GuestTransceiver::open(channel_ptr_t channel, bool server, descriptor_map_t preloadPublishTypes/* = {}*/, descriptor_map_t preloadSubscribeTypes/* = {}*/)
	{
		if (m_hostConnection != std::nullopt)
        {
            throw std::logic_error{ "already connected" };
        }

		m_preloadPublishTypes = std::move(preloadPublishTypes);
		m_preloadSubscribeTypes = std::move(preloadSubscribeTypes);
		
		m_hostConnection.emplace(std::move(channel), server);
		m_hostConnection->asyncReceive(registry(), selfName(),
			[this](io::Connection& connection, const DotsTransportHeader& header, Transmission&& transmission, bool isFromMyself){ return handleReceive(connection, header, std::move(transmission), isFromMyself); },
			[this](io::Connection& connection, const std::exception_ptr& e){ handleTransition(connection, e); }
		);
		
		return *m_hostConnection;
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

        try
        {
            m_hostConnection->transmit(instance, includedProperties, remove);
        }
        catch (...)
        {
			m_hostConnection->handleError(std::current_exception());
        }
	}

	void GuestTransceiver::joinGroup(const std::string_view& name)
	{
		LOG_DEBUG_S("send DotsMember (join " << name << ")");
		publish(DotsMember{
            DotsMember::groupName_i{ name },
            DotsMember::event_i{ DotsMemberEvent::join }
        });
	}

	void GuestTransceiver::leaveGroup(const std::string_view& name)
	{
		LOG_DEBUG_S("send DotsMember (leave " << name << ")");
		publish(DotsMember{
            DotsMember::groupName_i{ name },
            DotsMember::event_i{ DotsMemberEvent::leave }
        });
	}

	bool GuestTransceiver::handleReceive(io::Connection&/* connection*/, const DotsTransportHeader& header, Transmission&& transmission, bool isFromMyself)
	{
		dispatcher().dispatch(header.dotsHeader, transmission.instance(), isFromMyself);
        return true;
	}
	
	void GuestTransceiver::handleTransition(io::Connection& connection, const std::exception_ptr& e) noexcept
	{
        try
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
				    connection.transmit(DotsMember{
                        DotsMember::groupName_i{ name },
                        DotsMember::event_i{ DotsMemberEvent::join }
                    });
			    }

			    m_preloadPublishTypes.clear();
			    m_preloadSubscribeTypes.clear();
		    }
		    else if (connection.state() == DotsConnectionState::closed)
			{
		        if (e != nullptr)
		        {
					try
                    {
                        std::rethrow_exception(e);
                    }
                    catch (const std::exception& e)
                    {
						LOG_ERROR_S("connection error: " << e.what());
                    }
		        }

		        LOG_INFO_S("connection closed -> peerId: " << connection.peerId() << ", name: " << connection.peerName());
		        m_hostConnection = std::nullopt;
		    }
        }
        catch (const std::exception& e)
        {
			LOG_ERROR_S("error while handling connection transition -> peerId: " << connection.peerId() << ", name: " << connection.peerName() << " -> " << e.what());
			m_hostConnection = std::nullopt;
        }
	}
}