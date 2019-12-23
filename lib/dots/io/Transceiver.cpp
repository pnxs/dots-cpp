#include <dots/io/Transceiver.h>
#include <set>
#include <dots/common/logging.h>
#include <dots/io/serialization/AsciiSerialization.h>
#include <dots/io/DescriptorConverter.h>
#include <DotsMsgConnect.dots.h>
#include <DotsMember.dots.h>
#include <DotsCacheInfo.dots.h>
#include <DotsClient.dots.h>
#include <DotsDescriptorRequest.dots.h>

namespace dots
{
	Transceiver::Transceiver()
		: m_connectionState(DotsConnectionState::closed)
		, m_id(0)
	{
		/* do nothing */
	}

	bool Transceiver::openChannel(channel_ptr_t channel, std::string name, descriptor_map_t preloadPublishTypes/* = {}*/, descriptor_map_t preloadSubscribeTypes/* = {}*/)
	{
		if (m_connectionState != DotsConnectionState::closed)
		{
			throw std::logic_error{ "already receiving from an open channel" };
		}
		
		LOG_DEBUG_S("start transceiver");
		m_name = std::move(name);
		m_channel = std::move(channel);
		m_preloadPublishTypes = std::move(preloadPublishTypes);
		m_preloadSubscribeTypes = std::move(preloadSubscribeTypes);
		
		m_channel->asyncReceive(m_registry, 
			[this](const DotsTransportHeader& transportHeader, Transmission&& transmission){ return handleReceive(transportHeader, std::move(transmission)); },
			[this](const std::exception& e){ handleError(e); });
		setConnectionState(DotsConnectionState::connecting);

		return true;
	}

	void Transceiver::closeChannel()
	{
		m_channel.reset();
		setConnectionState(DotsConnectionState::closed);
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

	DotsConnectionState Transceiver::connectionState() const
	{
		return m_connectionState;
	}
	
	uint32_t Transceiver::id() const
	{
		return m_id;
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
		exportType(instance._descriptor());
		m_channel->transmit(instance, includedProperties, remove);
		
	}

	void Transceiver::remove(const type::Struct& instance)
	{
		publish(instance, instance._keyProperties(), true);
	}

	void Transceiver::publish(const type::StructDescriptor<>*/* td*/, const type::Struct& instance, types::property_set_t what, bool remove)
	{
		publish(instance, what, remove);
	}

	bool Transceiver::connected() const
	{
		return m_connectionState == DotsConnectionState::connected;
	}

	void Transceiver::joinGroup(const std::string_view& name)
	{
		LOG_DEBUG_S("send DotsMember (join " << name << ")");
		publish(DotsMember{
            DotsMember::groupName_i{ name },
            DotsMember::event_i{ DotsMemberEvent::join }
        });
	}

	void Transceiver::leaveGroup(const std::string_view& name)
	{
		LOG_INFO_S("send DotsMember (leave " << name << ")");
		publish(DotsMember{
            DotsMember::groupName_i{ name },
            DotsMember::event_i{ DotsMemberEvent::leave }
        });
	}

	bool Transceiver::handleReceive(const DotsTransportHeader& transportHeader, Transmission&& transmission)
	{
		try 
        {
			if (m_connectionState == DotsConnectionState::connecting)
			{
				if (auto* dotsMsgHello = transmission.instance()->_as<DotsMsgHello>())
                {
                    processHello(*dotsMsgHello);
                }
                else if (auto* dotsMsgConnectResponse = transmission.instance()->_as<DotsMsgConnectResponse>())
                {
                    processConnectResponse(*dotsMsgConnectResponse);
                }
				else
				{
					throw std::runtime_error{ "received unexpected instance during connecting: " + transmission.instance()->_descriptor().name() };
				}
			}
			else 
			{
				if (auto* dotsMsgConnectResponse = transmission.instance()->_as<DotsMsgConnectResponse>())
				{
					if (m_connectionState == DotsConnectionState::early_subscribe)
	                {
	                    processEarlySubscribe(*dotsMsgConnectResponse);
	                }
					else
					{
						throw std::runtime_error{ "received unexpected DotsMsgConnectResponse during early_subscribe" };
					}
				}
				else
				{
					importType(transmission.instance());
					DotsHeader dotsHeader = transportHeader.dotsHeader;
	                dotsHeader.isFromMyself(dotsHeader.sender == m_id);
	                m_dispatcher.dispatch(dotsHeader, transmission.instance());
				}
			}

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
		LOG_ERROR_S("channel error in async receive: " << e.what());
		closeChannel();
	}

	void Transceiver::processHello(const DotsMsgHello& hello)
	{
		if (hello.authChallenge.isValid() && hello.serverName.isValid())
		{
			LOG_DEBUG_S("received hello from '" << *hello.serverName << "' authChallenge=" << hello.authChallenge);
			LOG_DATA_S("send DotsMsgConnect");

			publish(DotsMsgConnect{
                DotsMsgConnect::clientName_i{ m_name },
                DotsMsgConnect::preloadCache_i{ true }
            });
		}
		else
		{
			LOG_WARN_S("Invalid hello from server valatt:" << hello._validProperties().toString());
		}
	}
	
	void Transceiver::processConnectResponse(const DotsMsgConnectResponse& connectResponse)
	{
		const std::string& serverName = connectResponse.serverName.isValid() ? *connectResponse.serverName : "<unknown>";
		LOG_DEBUG_S("connectResponse: serverName=" << serverName << " accepted=" << *connectResponse.accepted);
		
		if (connectResponse.clientId.isValid())
		{
			m_id = connectResponse.clientId;
		}
		
		if (connectResponse.preload == true && (!connectResponse.preloadFinished.isValid() || connectResponse.preloadFinished == false))
		{
			setConnectionState(DotsConnectionState::early_subscribe);
			
			for (const auto& [name, descriptor] : m_preloadPublishTypes)
			{
				(void)name;
				exportType(*descriptor);
			}

			for (const auto& [name, descriptor] : m_preloadSubscribeTypes)
			{
				exportType(*descriptor);			
				joinGroup(name);
			}

			publish(DotsMsgConnect{
				DotsMsgConnect::preloadClientFinished_i{ true }
			});

			m_preloadPublishTypes.clear();
			m_preloadSubscribeTypes.clear();
		}
		else
		{
			setConnectionState(DotsConnectionState::connected);
		}
	}
	
	void Transceiver::processEarlySubscribe(const DotsMsgConnectResponse& connectResponse)
	{
		if (connectResponse.preloadFinished == true)
        {
            setConnectionState(DotsConnectionState::connected);
        }
        else
        {
            LOG_ERROR_S("invalid DotsMsgConnectResponse");
        }
	}

	void Transceiver::setConnectionState(DotsConnectionState state)
	{
		LOG_DEBUG_S("change connection state to " << to_string(state));
		m_connectionState = state;
	}

	void Transceiver::importType(const type::Struct& instance)
    {
        if (auto* structDescriptorData = instance._as<StructDescriptorData>())
        {
        	if (bool isNewSharedType = m_sharedTypes.emplace(structDescriptorData->name).second; isNewSharedType)
        	{
        		io::DescriptorConverter{ m_registry }(*structDescriptorData);
        	}
        }
        else if (auto* enumDescriptorData = instance._as<EnumDescriptorData>())
        {
        	if (bool isNewSharedType = m_sharedTypes.emplace(enumDescriptorData->name).second; isNewSharedType)
        	{
        		io::DescriptorConverter{ m_registry }(*enumDescriptorData);
        	}
        }
    }

    void Transceiver::exportType(const type::Descriptor<>& descriptor)
    {
        if (bool isNewSharedType = m_sharedTypes.emplace(descriptor.name()).second; isNewSharedType)
    	{
    		if (descriptor.type() == type::Type::Enum)
		    {
			    auto& enumDescriptor = static_cast<const type::EnumDescriptor<>&>(descriptor);
    			exportType(enumDescriptor.underlyingDescriptor());
	    		publish(io::DescriptorConverter{ m_registry }(enumDescriptor));
		    }
	        else if (descriptor.type() == type::Type::Struct)
	        {
	        	if (auto& structDescriptor = static_cast<const type::StructDescriptor<>&>(descriptor); !structDescriptor.internal())
	        	{
	        		for (const type::PropertyDescriptor& propertyDescriptor : structDescriptor.propertyDescriptors())
    				{
    					exportType(propertyDescriptor.valueDescriptor());
    				}

    				publish(io::DescriptorConverter{ m_registry }(structDescriptor));
	        	}
	        }
    	}
    }
}