#include <dots/io/Connection.h>
#include <dots/io/Registry.h>
#include <dots/io/DescriptorConverter.h>
#include <DotsMsgConnect.dots.h>
#include <DotsCacheInfo.dots.h>
#include <DotsClient.dots.h>
#include <DotsDescriptorRequest.dots.h>

namespace dots::io
{
	Connection::Connection(channel_ptr_t channel, bool server) :
        m_expectedSystemType{ &DotsMsgError::_Descriptor(), types::property_set_t::None, nullptr },
        m_connectionState(DotsConnectionState::suspended),
        m_selfId(server ? ServerId : UninitializedId),
		m_peerId(server ? M_nextClientId++ : ServerId),
	    m_peerName("<not_set>"),
		m_channel(std::move(channel)),
		m_registry(nullptr)
	{
		/* do nothing */
	}

    Connection::~Connection()
    {
        if (m_connectionState == DotsConnectionState::connected)
        {
            transmit(DotsMsgError{ DotsMsgError::errorCode_i{ 0 } });
        }
    }

    DotsConnectionState Connection::state() const
	{
		return m_connectionState;
	}

    id_t Connection::selfId() const
    {
        return m_selfId;
    }

    id_t Connection::peerId() const
	{
		return m_peerId;
	}

    const std::string& Connection::peerName() const
    {
		return m_peerName;
    }

    bool Connection::connected() const
	{
		return m_connectionState == DotsConnectionState::connected;
	}

	void Connection::asyncReceive(Registry& registry, const std::string_view& name, receive_handler_t&& receiveHandler, transition_handler_t&& transitionHandler)
	{
		if (m_connectionState != DotsConnectionState::suspended)
        {
            throw std::logic_error{ "only one async receive can be started on a connection" };
        }

        if (receiveHandler == nullptr || transitionHandler == nullptr)
        {
            throw std::logic_error{ "both a receive and a transition must be set" };
        }

		m_registry = &registry;
		m_receiveHandler = std::move(receiveHandler);
		m_transitionHandler = std::move(transitionHandler);
		
		setConnectionState(DotsConnectionState::connecting);
		m_channel->asyncReceive(registry,
			[this](const DotsTransportHeader& transportHeader, Transmission&& transmission){ return handleReceive(transportHeader, std::move(transmission)); },
			[this](const std::exception& e){ handleError(e); }
		);

		if (m_selfId == ServerId)
		{
		    transmit(DotsMsgHello{
                DotsMsgHello::serverName_i{ name },
                DotsMsgHello::authChallenge_i{ 0 }
            });

            expectSystemType<DotsMsgConnect>(DotsMsgConnect::clientName_p + DotsMsgConnect::preloadCache_p, &Connection::handleConnect);
		}
        else
        {
            transmit(DotsMsgConnect{
                DotsMsgConnect::clientName_i{ name },
                DotsMsgConnect::preloadCache_i{ true }
            });

            expectSystemType<DotsMsgHello>(DotsMsgHello::serverName_p + DotsMsgHello::authChallenge_p, &Connection::handleHello);
        }
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
				DotsHeader::sender_i{ m_selfId },
                DotsHeader::removeObj_i{ remove }
            }
        };

        if (descriptor.internal() && !instance._is<DotsClient>() && !instance._is<DotsDescriptorRequest>())
        {
            header.nameSpace("SYS");
        }

		transmit(header, instance);
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

    void Connection::transmit(const type::StructDescriptor<>& descriptor)
    {
        exportType(descriptor);
    }

    bool Connection::handleReceive(const DotsTransportHeader& transportHeader, Transmission&& transmission)
	{
        if (m_connectionState == DotsConnectionState::closed)
        {
            return false;
        }

        try 
        {
            const type::Struct& instance = transmission.instance();

            if (instance._isAny<DotsMsgHello, DotsMsgConnect, DotsMsgConnectResponse, DotsMsgError>())
            {
                if (auto* dotsMsgError = instance._as<DotsMsgError>())
                {
                    handlePeerError(*dotsMsgError);
                    return false;
                }
                else
                {
                    const auto& [expectedType, expectedProperties, handler] = m_expectedSystemType;
                    instance._assertIs(expectedType);
                    instance._assertHasProperties(expectedProperties); // note: subsets are allowed for backwards compatibility with old implementation

                    handler(instance);
                }
            }
            else
            {
                if (m_connectionState == DotsConnectionState::connected || m_connectionState == DotsConnectionState::early_subscribe)
                {
                    importType(transmission.instance());

                    if (m_selfId == ServerId)
                    {
                        DotsTransportHeader transportHeader_ = transportHeader;
                        DotsHeader& dotsHeader = *transportHeader_.dotsHeader;
                        dotsHeader.sender = m_peerId;

                        dotsHeader.serverSentTime = pnxs::SystemNow();

                        if (!dotsHeader.sentTime.isValid())
                        {
                            dotsHeader.sentTime = dotsHeader.serverSentTime;
                        }

                        m_receiveHandler(*this, transportHeader_, std::move(transmission), transportHeader_.dotsHeader->sender == m_selfId);
                    }
                    else
                    {
                        m_receiveHandler(*this, transportHeader, std::move(transmission), transportHeader.dotsHeader->sender == m_selfId);
                    }
                }
                else
                {
                    throw std::logic_error{ "received instance of non-system type " + instance._descriptor().name() + " while not in early_subscribe or connected state " + to_string(m_connectionState) };
                }
            }

            return true;
        }
        catch (const std::exception& e) 
        {
            transmit(DotsMsgError{
                DotsMsgError::errorCode_i{ 1 },
                DotsMsgError::errorText_i{ e.what() }
            });
            
            handleError(e);
            return false;
        }
	}

	void Connection::handleError(const std::exception& e)
	{
        if (m_connectionState == DotsConnectionState::closed)
        {
            throw std::runtime_error{ "unable to handle error in closed state: " + std::string{ e.what() } };
        }

        handleClose(&e);
	}

    void Connection::handleClose(const std::exception* e)
	{
	    m_receiveHandler = nullptr;
        m_registry = nullptr;
        expectSystemType<DotsMsgError>(types::property_set_t::None, nullptr);
        setConnectionState(DotsConnectionState::closed, e);
	}

	void Connection::handleHello(const DotsMsgHello& hello)
	{
		m_peerName = hello.serverName;
		LOG_DEBUG_S("received hello from '" << *hello.serverName << "' authChallenge=" << hello.authChallenge);

        expectSystemType<DotsMsgConnectResponse>(DotsMsgConnectResponse::clientId_p + DotsMsgConnectResponse::accepted_p + DotsMsgConnectResponse::preload_p, &Connection::handleAuthorizationRequest);
	}
	
	void Connection::handleAuthorizationRequest(const DotsMsgConnectResponse& connectResponse)
	{
        m_peerId = connectResponse.clientId;
		LOG_DEBUG_S("connectResponse: serverName=" << m_peerName << " accepted=" << *connectResponse.accepted);
		
		if (connectResponse.preload == true)
		{
			setConnectionState(DotsConnectionState::early_subscribe);
            transmit(DotsMsgConnect{ DotsMsgConnect::preloadClientFinished_i{ true } });
            expectSystemType<DotsMsgConnectResponse>(DotsMsgConnectResponse::preloadFinished_p, &Connection::handlePreloadFinished);
		}
		else
		{
			setConnectionState(DotsConnectionState::connected);
            expectSystemType<DotsMsgError>(DotsMsgError::errorCode_p, &Connection::handlePeerError);
		}
	}
	
	void Connection::handlePreloadFinished(const DotsMsgConnectResponse&/* connectResponse*/)
	{
		setConnectionState(DotsConnectionState::connected);
        expectSystemType<DotsMsgError>(DotsMsgError::errorCode_p, &Connection::handlePeerError);
	}

    void Connection::handleConnect(const DotsMsgConnect& connect)
    {
        m_peerName = connect.clientName;

        LOG_INFO_S("authorized");

        transmit(DotsMsgConnectResponse{
            DotsMsgConnectResponse::clientId_i{ m_peerId },
            DotsMsgConnectResponse::preload_i{ connect.preloadCache == true },
            DotsMsgConnectResponse::accepted_i{ true },
        });

        if (connect.preloadCache == true)
        {
            setConnectionState(DotsConnectionState::early_subscribe);
            expectSystemType<DotsMsgConnect>(DotsMsgConnect::preloadClientFinished_p, &Connection::handlePreloadClientFinished);
        }
        else
        {
            setConnectionState(DotsConnectionState::connected);
            expectSystemType<DotsMsgError>(DotsMsgError::errorCode_p, &Connection::handlePeerError);
        }
    }

    void Connection::handlePreloadClientFinished(const DotsMsgConnect& connect)
    {
        // Check authentication and authorization;
        if (connect.preloadClientFinished == false)
        {
            throw std::logic_error{ "expected preload client finished to be true" };
        }

        setConnectionState(DotsConnectionState::connected);

        // When all cache items are sent to client, send fin-message
        transmit(DotsMsgConnectResponse{
            DotsMsgConnectResponse::preloadFinished_i{ true }
        });

        expectSystemType<DotsMsgError>(DotsMsgError::errorCode_p, &Connection::handlePeerError);
    }

    void Connection::handlePeerError(const DotsMsgError& error)
    {
        if (error.errorCode == 0)
        {
            handleClose(nullptr);
        }
        else
        {
            std::string what = "received DOTS error: (";
            what += error.errorCode.isValid() ? std::to_string(error.errorCode) : std::string{ "<unknown error code>" };
            what += ") ";
            what += error.errorText.isValid() ? *error.errorText : std::string{ "<unknown error>" };
            handleError(std::runtime_error{ what });
        }
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

	void Connection::setConnectionState(DotsConnectionState state, const std::exception* e/* = nullptr*/)
	{
		LOG_DEBUG_S("change connection state to " << to_string(state));
		m_connectionState = state;
        m_transitionHandler(*this, e);
	}

    template <typename T>
    void Connection::expectSystemType(const types::property_set_t& expectedAttributes, void(Connection::* handler)(const T&))
    {
        m_expectedSystemType = { &T::_Descriptor(), expectedAttributes, [this, handler](const type::Struct& instance){ (this->*handler)(instance._to<T>()); } };
    }
}