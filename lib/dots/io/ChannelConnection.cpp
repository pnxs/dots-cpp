#include <dots/io/ChannelConnection.h>
#include <dots/io/Registry.h>
#include <dots/io/DescriptorConverter.h>
#include <DotsMsgConnect.dots.h>
#include <DotsMember.dots.h>
#include <DotsCacheInfo.dots.h>
#include <DotsClient.dots.h>
#include <DotsDescriptorRequest.dots.h>

namespace dots::io
{
	ChannelConnection::ChannelConnection(channel_ptr_t channel, descriptor_map_t preloadPublishTypes/* = {}*/, descriptor_map_t preloadSubscribeTypes/* = {}*/) :
		m_connectionState(DotsConnectionState::closed),
		m_id(0),
		m_channel(std::move(channel)),
		m_registry(nullptr),
		m_preloadPublishTypes(std::move(preloadPublishTypes)),
		m_preloadSubscribeTypes(preloadSubscribeTypes)
	{
		/* do nothing */
	}

	DotsConnectionState ChannelConnection::state() const
	{
		return m_connectionState;
	}
	
	uint32_t ChannelConnection::id() const
	{
		return m_id;
	}

	bool ChannelConnection::connected() const
	{
		return m_connectionState == DotsConnectionState::connected;
	}

	void ChannelConnection::asyncReceive(Registry& registry, const std::string& name, receive_handler_t&& receiveHandler, error_handler_t&& errorHandler)
	{
		if (m_connectionState != DotsConnectionState::closed)
        {
            throw std::logic_error{ "only one async receive can be active at the same time" };
        }

		m_registry = &registry;
		m_receiveHandler = std::move(receiveHandler);
		m_errorHandler = std::move(errorHandler);
		
		setConnectionState(DotsConnectionState::connecting);
		m_channel->asyncReceive(registry,
			[this](const DotsTransportHeader& transportHeader, Transmission&& transmission){ return handleReceive(transportHeader, std::move(transmission)); },
			[this](const std::exception& e){ handleError(e); }
		);

		transmit(DotsMsgConnect{
            DotsMsgConnect::clientName_i{ m_name },
            DotsMsgConnect::preloadCache_i{ true }
        });
	}

	void ChannelConnection::transmit(const type::Struct& instance, types::property_set_t includedProperties, bool remove)
	{
		exportType(instance._descriptor());
		m_channel->transmit(instance, includedProperties, remove);
	}

	void ChannelConnection::joinGroup(const std::string_view& name)
	{
		LOG_DEBUG_S("send DotsMember (join " << name << ")");
		transmit(DotsMember{
            DotsMember::groupName_i{ name },
            DotsMember::event_i{ DotsMemberEvent::join }
        });
	}

	void ChannelConnection::leaveGroup(const std::string_view& name)
	{
		LOG_INFO_S("send DotsMember (leave " << name << ")");
		transmit(DotsMember{
            DotsMember::groupName_i{ name },
            DotsMember::event_i{ DotsMemberEvent::leave }
        });
	}

	bool ChannelConnection::handleReceive(const DotsTransportHeader& transportHeader, Transmission&& transmission)
	{
		try 
        {
            if (transportHeader.nameSpace == "SYS")
            {
                handleControlMessage(transportHeader, std::move(transmission));
            } 
            else
            {
                handleRegularMessage(transportHeader, std::move(transmission));
            }

            return true;
        }
        catch (const std::exception& e) 
        {
            handleError(e);
            return false;
        }
	}

	void ChannelConnection::handleControlMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission)
	{
        switch(m_connectionState)
        {
            case DotsConnectionState::connecting:
                if (auto* dotsMsgHello = transmission.instance()->_as<DotsMsgHello>())
                {
                    processHello(*dotsMsgHello);
                }
                else if (auto* dotsMsgConnectResponse = transmission.instance()->_as<DotsMsgConnectResponse>())
                {
                    processConnectResponse(*dotsMsgConnectResponse);
                }
                break;
            case DotsConnectionState::early_subscribe:
                if (auto* dotsMsgConnectResponse = transmission.instance()->_as<DotsMsgConnectResponse>())
                {
                    processEarlySubscribe(*dotsMsgConnectResponse);
                }
                [[fallthrough]];
            case DotsConnectionState::connected:
                {
                    if (transmission.instance()->_is<DotsCacheInfo>()) 
                    {
                        // TODO: implement handling of DotsCacheInfo
                        // for now let trough like an normal object
                    }

            		importType(transmission.instance());
                    handleRegularMessage(transportHeader, std::move(transmission));
                }

                break;
            case DotsConnectionState::suspended:
                // buffer outgoing messages
                break;
            case DotsConnectionState::closed:
                // do nothing
                break;
        }
	}
	
	void ChannelConnection::handleRegularMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission)
	{
		switch(m_connectionState)
        {
            case DotsConnectionState::connecting:
                break;
            case DotsConnectionState::early_subscribe:
            case DotsConnectionState::connected:
                {
					DotsTransportHeader transportHeader_ = transportHeader;
                    DotsHeader dotsHeader = transportHeader_.dotsHeader;
                    dotsHeader.isFromMyself(dotsHeader.sender == m_id);
                    m_receiveHandler(transportHeader_, std::move(transmission));
                }
                break;
            case DotsConnectionState::suspended:
                // buffer outgoing messages
                break;
            case DotsConnectionState::closed:
                // do nothing
                break;
        }
	}

	void ChannelConnection::handleError(const std::exception& e)
	{
		LOG_ERROR_S("channel error in async receive: " << e.what());
		m_registry = nullptr;
		m_receiveHandler = nullptr;
		m_errorHandler = nullptr;
		setConnectionState(DotsConnectionState::closed);
	}

	void ChannelConnection::processHello(const DotsMsgHello& hello)
	{
		if (hello.authChallenge.isValid() && hello.serverName.isValid())
		{
			LOG_DEBUG_S("received hello from '" << *hello.serverName << "' authChallenge=" << hello.authChallenge);
		}
		else
		{
			LOG_WARN_S("Invalid hello from server valatt:" << hello._validProperties().toString());
		}
	}
	
	void ChannelConnection::processConnectResponse(const DotsMsgConnectResponse& connectResponse)
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

			transmit(DotsMsgConnect{
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
	
	void ChannelConnection::processEarlySubscribe(const DotsMsgConnectResponse& connectResponse)
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
	
	void ChannelConnection::importType(const type::Struct& instance)
    {
        if (auto* structDescriptorData = instance._as<StructDescriptorData>())
        {
        	if (bool isNewSharedType = m_sharedTypes.emplace(structDescriptorData->name).second; isNewSharedType)
        	{
        		DescriptorConverter{ *m_registry }(*structDescriptorData);
        	}
        }
        else if (auto* enumDescriptorData = instance._as<EnumDescriptorData>())
        {
        	if (bool isNewSharedType = m_sharedTypes.emplace(enumDescriptorData->name).second; isNewSharedType)
        	{
        		DescriptorConverter{ *m_registry }(*enumDescriptorData);
        	}
        }
    }

    void ChannelConnection::exportType(const type::Descriptor<>& descriptor)
    {
        if (bool isNewSharedType = m_sharedTypes.emplace(descriptor.name()).second; isNewSharedType)
    	{
    		if (descriptor.type() == type::Type::Enum)
		    {
			    auto& enumDescriptor = static_cast<const type::EnumDescriptor<>&>(descriptor);
    			exportType(enumDescriptor.underlyingDescriptor());
	    		transmit(DescriptorConverter{ *m_registry }(enumDescriptor));
		    }
	        else if (descriptor.type() == type::Type::Struct)
	        {
	        	if (auto& structDescriptor = static_cast<const type::StructDescriptor<>&>(descriptor); !structDescriptor.internal())
	        	{
	        		for (const type::PropertyDescriptor& propertyDescriptor : structDescriptor.propertyDescriptors())
    				{
    					exportType(propertyDescriptor.valueDescriptor());
    				}

    				transmit(DescriptorConverter{ *m_registry }(structDescriptor));
	        	}
	        }
    	}
    }

	void ChannelConnection::setConnectionState(DotsConnectionState state)
	{
		LOG_DEBUG_S("change connection state to " << to_string(state));
		m_connectionState = state;
	}
}