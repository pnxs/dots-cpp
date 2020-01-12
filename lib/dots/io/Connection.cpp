#include <dots/io/Connection.h>
#include <dots/io/Registry.h>
#include <dots/io/DescriptorConverter.h>
#include <DotsMsgConnect.dots.h>
#include <DotsMember.dots.h>
#include <DotsCacheInfo.dots.h>
#include <DotsClient.dots.h>
#include <DotsDescriptorRequest.dots.h>
#include <DotsMsgError.dots.h>

namespace dots::io
{
	Connection::Connection(channel_ptr_t channel, bool server, descriptor_map_t preloadPublishTypes/* = {}*/, descriptor_map_t preloadSubscribeTypes/* = {}*/) :
		m_server(server),
        m_connectionState(DotsConnectionState::closed),
		m_id(m_server ? M_nextClientId++ : 0),
		m_channel(std::move(channel)),
		m_registry(nullptr),
	    m_name("<not_set>"),
		m_preloadPublishTypes(std::move(preloadPublishTypes)),
		m_preloadSubscribeTypes(std::move(preloadSubscribeTypes))
	{
		/* do nothing */
	}

	DotsConnectionState Connection::state() const
	{
		return m_connectionState;
	}
	
	id_t Connection::id() const
	{
		return m_id;
	}

    const std::string& Connection::name() const
    {
		return m_name;
    }

    bool Connection::connected() const
	{
		return m_connectionState == DotsConnectionState::connected;
	}

	void Connection::asyncReceive(Registry& registry, const std::string& name, receive_handler_t&& receiveHandler, error_handler_t&& errorHandler)
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

		if (m_server)
		{
		    transmit(DotsMsgHello{
                DotsMsgHello::serverName_i{ name },
                DotsMsgHello::authChallenge_i{ 0 }
            });

			return;
		}
		
		transmit(DotsMsgConnect{
            DotsMsgConnect::clientName_i{ name },
            DotsMsgConnect::preloadCache_i{ true }
        });
	}

	void Connection::transmit(const type::Struct& instance, types::property_set_t includedProperties, bool remove)
	{
		const type::StructDescriptor<>& descriptor = instance._descriptor();
		exportType(descriptor);

        DotsTransportHeader header{
            DotsTransportHeader::destinationGroup_i{ descriptor.name() },
            DotsTransportHeader::dotsHeader_i{
                DotsHeader::typeName_i{ descriptor.name() },
                DotsHeader::sentTime_i{ pnxs::SystemNow() },
                DotsHeader::attributes_i{ includedProperties ==  types::property_set_t::All ? instance._validProperties() : includedProperties },
				DotsHeader::sender_i{ m_server ? ServerId : m_id },
                DotsHeader::removeObj_i{ remove }
            }
        };

        if (descriptor.internal() && !instance._is<DotsClient>() && !instance._is<DotsDescriptorRequest>())
        {
            header.nameSpace("SYS");
        }

		m_channel->transmit(header, instance);
	}

    void Connection::transmit(const DotsTransportHeader& header, const type::Struct& instance)
    {
        try
        {
            m_channel->transmit(header, instance);
        }
        catch (const std::exception& e)
        {
            handleError(e);
        }
    }

    void Connection::transmit(const DotsTransportHeader& header, const Transmission& transmission)
    {
        try
        {
            m_channel->transmit(header, transmission);
        }
        catch (const std::exception& e)
        {
           handleError(e);
        }
    }

	void Connection::joinGroup(const std::string_view& name)
	{
		LOG_DEBUG_S("send DotsMember (join " << name << ")");
		transmit(DotsMember{
            DotsMember::groupName_i{ name },
            DotsMember::event_i{ DotsMemberEvent::join }
        });
	}

	void Connection::leaveGroup(const std::string_view& name)
	{
		LOG_INFO_S("send DotsMember (leave " << name << ")");
		transmit(DotsMember{
            DotsMember::groupName_i{ name },
            DotsMember::event_i{ DotsMemberEvent::leave }
        });
	}

	bool Connection::handleReceive(const DotsTransportHeader& transportHeader, Transmission&& transmission)
	{
		if (m_server)
		{
		    return handleReceiveServer(transportHeader, std::move(transmission));
		}

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

	void Connection::handleControlMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission)
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
	
	void Connection::handleRegularMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission)
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

	void Connection::handleError(const std::exception& e)
	{
        if (m_server)
        {
            LOG_ERROR_S("channel error in async receive: " << e.what());
            error_handler_t errorHandler;
            errorHandler.swap(m_errorHandler);

            m_registry = nullptr;
		    m_receiveHandler = nullptr;
		    setConnectionState(DotsConnectionState::closed);

            errorHandler(m_id, e);

            return;
        }

		LOG_ERROR_S("channel error in async receive: " << e.what());
		m_registry = nullptr;
		m_receiveHandler = nullptr;
		m_errorHandler = nullptr;
		setConnectionState(DotsConnectionState::closed);
	}

	void Connection::processHello(const DotsMsgHello& hello)
	{
		if (hello.authChallenge.isValid() && hello.serverName.isValid())
		{
			LOG_DEBUG_S("received hello from '" << *hello.serverName << "' authChallenge=" << hello.authChallenge);
			m_name = hello.serverName;
		}
		else
		{
			LOG_WARN_S("Invalid hello from server valatt:" << hello._validProperties().toString());
		}
	}
	
	void Connection::processConnectResponse(const DotsMsgConnectResponse& connectResponse)
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
	
	void Connection::processEarlySubscribe(const DotsMsgConnectResponse& connectResponse)
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

	bool Connection::handleReceiveServer(const DotsTransportHeader& transportHeader, Transmission&& transmission)
    {
        LOG_DEBUG_S("handleReceive:");
        bool handled = false;

        auto modifiedHeader = transportHeader;
        // Overwrite sender to known client peerAddress
        auto& dotsHeader = *modifiedHeader.dotsHeader;
        dotsHeader.sender = id();

        dotsHeader.serverSentTime = pnxs::SystemNow();

        if (!dotsHeader.sentTime.isValid())
        {
            dotsHeader.sentTime = dotsHeader.serverSentTime;
        }

        try
        {
            // Check for DOTS control message-types
            if (transportHeader.nameSpace.isValid() && *transportHeader.nameSpace == "SYS")
            {
                handled = handleControlMessageServer(modifiedHeader, std::move(transmission));
            }
            else
            {
                handled = handleRegularMessageServer(modifiedHeader, std::move(transmission));
            }

            if (!handled)
            {
                string objName;
                if (transportHeader.nameSpace.isValid()) objName = "::" + *transportHeader.nameSpace + "::";
                objName += *transportHeader.destinationGroup;
                string errorText = "invalid message received while in state " + to_string(m_connectionState) + ": " + objName;
                LOG_WARN_S(errorText);
                // send false response;

                transmit(DotsMsgError{
                    DotsMsgError::errorCode_i{ 1 },
                    DotsMsgError::errorText_i{ errorText }
                });
            }
        }
        catch (const std::exception& e)
        {
            string errorReport = "exception in receive [";
            errorReport += "dstGrp=" + *transportHeader.destinationGroup;
            errorReport += ",state=" + to_string(m_connectionState);
            errorReport += string("]:") + e.what();

            LOG_ERROR_S(errorReport);

            transmit(DotsMsgError{
                DotsMsgError::errorCode_i{ 2 },
                DotsMsgError::errorText_i{ errorReport }
            });

            handleError(e);
        }

        return m_connectionState != DotsConnectionState::closed;
    }

    /**
     *
     * @code
     * Receive-Table:
     * *State                    | connecting | early_subscribe | connected | suspended | closed
     * --------------------------+------------+-----------------+-----------+-----------+---------
     * SYS::DotsMsgConnect       |     X      |                 |           |           |
     * SYS::DotsMember           |            |        X        |     X     |           |
     * SYS::EnumDescriptorData   |            |        X        |     X     |           |
     * SYS::StructDescriptorData |            |        X        |     X     |           |
     *                           |            |                 |           |           |
     *                           |            |                 |           |           |
     *                           |            |                 |           |           |
     *                           |            |                 |           |           |
     *                           |            |                 |           |           |
     *                           |            |                 |           |           |
     * *Other*                   |            |                 |     X     |           |
     *                           |            |                 |           |           |
     *                           |            |                 |           |           |
     * @endcode
     */
    bool Connection::handleControlMessageServer(const DotsTransportHeader& transportHeader, Transmission&& transmission)
    {
        bool handled = false;

        switch (m_connectionState)
        {
            case DotsConnectionState::connecting:
                // Only accept DotsMsgConnect messages (MsgType connect)
                if (auto* dotsMsgConnect = transmission.instance()->_as<DotsMsgConnect>())
                {
                    // Check authentication and authorization;
                    processConnectRequest(*dotsMsgConnect);
                    handled = true;
                }
                break;
            case DotsConnectionState::early_subscribe:
                if (auto* dotsMsgConnect = transmission.instance()->_as<DotsMsgConnect>())
                {
                    // Check authentication and authorization;
                    processConnectPreloadClientFinished(*dotsMsgConnect);
                    handled = true;
                }
                [[fallthrough]];
            case DotsConnectionState::connected:
                importType(transmission.instance());
                m_receiveHandler(transportHeader, std::move(transmission));
                handled = true;
                break;
            case DotsConnectionState::suspended:
                LOG_WARN_S("state suspended not implemented");
                // Connection is temporarly not available
                break;

            case DotsConnectionState::closed:
                LOG_WARN_S("state closed not implemented");
                // Connection is closed and will never be open again
                break;
        }

        return handled;
    }

    bool Connection::handleRegularMessageServer(const DotsTransportHeader& transportHeader, Transmission&& transmission)
    {
        bool handled = false;
        switch (m_connectionState)
        {
            case DotsConnectionState::connecting:
            case DotsConnectionState::early_subscribe: // Only accept DotsMsgConnect messages (MsgType connect)
                break;

            case DotsConnectionState::connected:
                {
                    // Normal operation
                    m_receiveHandler(transportHeader, std::move(transmission));
                    handled = true;
                }
                break;

            case DotsConnectionState::suspended:
                LOG_WARN_S("state suspended not implemented");
                // Connection is temporarily not available
                break;

            case DotsConnectionState::closed:
                LOG_WARN_S("state closed not implemented");
                // Connection is closed and will never be open again
                break;
        }
        return handled;
    }

    void Connection::processConnectRequest(const DotsMsgConnect& msg)
    {
        m_name = msg.clientName;

        LOG_INFO_S("authorized");
        // Send DotsClient when Client is added to network.
        DotsClient{
            DotsClient::id_i{ m_id },
            DotsClient::name_i{ m_name },
            DotsClient::connectionState_i{ m_connectionState }
        }._publish();

        DotsMsgConnectResponse connectResponse{
            DotsMsgConnectResponse::accepted_i{ true },
            DotsMsgConnectResponse::clientId_i{ m_id }
        };

        if (msg.preloadCache == true)
        {
            connectResponse.preload(true);
        }
        transmit(connectResponse);

        if (msg.preloadCache == true)
        {
            setConnectionState(DotsConnectionState::early_subscribe);
        }
        else
        {
            setConnectionState(DotsConnectionState::connected);
        }
    }

    void Connection::processConnectPreloadClientFinished(const DotsMsgConnect& msg)
    {
        // Check authentication and authorization;
        if (!msg.preloadClientFinished.isValid() || msg.preloadClientFinished == false)
        {
            LOG_WARN_S("invalid DotsMsgConnect in state early_connect");
            return;
        }

        setConnectionState(DotsConnectionState::connected);

        // When all cache items are sent to client, send fin-message
        transmit(DotsMsgConnectResponse{
            DotsMsgConnectResponse::preloadFinished_i{ true }
        });
    }
	
	void Connection::importType(const type::Struct& instance)
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

    void Connection::exportType(const type::Descriptor<>& descriptor)
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

	void Connection::setConnectionState(DotsConnectionState state)
	{
		LOG_DEBUG_S("change connection state to " << to_string(state));
		m_connectionState = state;

        if (m_server)
        {
            DotsClient{ DotsClient::id_i{ id() }, DotsClient::connectionState_i{ state } }._publish();
        }
	}
}