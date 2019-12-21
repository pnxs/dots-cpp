#include "Transceiver.h"
#include <set>
#include "DotsMsgConnect.dots.h"
#include <dots/common/logging.h>
#include <dots/io/serialization/AsciiSerialization.h>
#include <DotsMember.dots.h>
#include <DotsCacheInfo.dots.h>
#include <DotsClient.dots.h>
#include <DotsDescriptorRequest.dots.h>

namespace dots
{
	Publisher* onPublishObject = nullptr;

	Transceiver::Transceiver()
		: m_connectionState(DotsConnectionState::closed)
		, m_id(0)
	{
		onPublishObject = this;
	}

	bool Transceiver::start(std::string name, channel_ptr_t channel, descriptor_map_t preloadPublishTypes/* = {}*/, descriptor_map_t preloadSubscribeTypes/* = {}*/)
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
		
		m_channel->asyncReceive(m_registry, [this](const DotsTransportHeader& transportHeader, Transmission&& transmission){ return handleReceive(transportHeader, std::move(transmission)); }, nullptr);
		setConnectionState(DotsConnectionState::connecting);

		return true;
	}

	void Transceiver::stop()
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
			throw std::logic_error{ "attempt to subscribe to substruct-only type" };
		}

		joinGroup(descriptor.name());
		return m_dispatcher.subscribe(descriptor, std::move(handler));
	}

	Subscription Transceiver::subscribe(const type::StructDescriptor<>& descriptor, event_handler_t<>&& handler)
	{
		if (descriptor.substructOnly())
		{
			throw std::logic_error{ "attempt to subscribe to substruct-only type" };
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
		publish(&instance._descriptor(), instance, includedProperties, remove);
	}
	
	void Transceiver::publish(const type::StructDescriptor<>* td, const type::Struct& instance, types::property_set_t what, bool remove)
	{
		if (td->substructOnly())
		{
			throw std::logic_error{ "attempt to publish substruct-only type" };
		}

		const type::StructDescriptor<>& descriptor = instance._descriptor();

    	if (!(descriptor.keyProperties() <= what))
	    {
	        throw std::runtime_error("tried to publish instance with invalid key (not all key-fields are set) what=" + what.toString() + " tdkeys=" + descriptor.keyProperties().toString());
	    }
		
		if (remove)
	    {
	        what ^= descriptor.keyProperties();
	    }
		
		LOG_DATA_S("data:" << to_ascii(&descriptor, &instance, what));
		m_channel->transmit(instance);
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

	void Transceiver::onEarlySubscribe()
	{
		TD_Traversal traversal;

		for (const auto& [name, descriptor] : m_preloadPublishTypes)
		{
			if (!descriptor->internal())
			{
				traversal.traverseDescriptorData(descriptor, [this](auto td, auto body) 
				{
					publish(*reinterpret_cast<const type::Struct*>(body), td->validProperties(body), false);
				});
			}
		}

		for (const auto& [name, descriptor] : m_preloadSubscribeTypes)
		{
			(void)name;
			
			if (!descriptor->internal())
			{
				traversal.traverseDescriptorData(descriptor, [this](auto td, auto body) 
				{
					publish(*reinterpret_cast<const type::Struct*>(body), td->validProperties(body), false);
				});
			}
			
			joinGroup(descriptor->name());
		}

		publish(DotsMsgConnect{
			DotsMsgConnect::preloadClientFinished_i{ true }
		});
	}

	bool Transceiver::handleReceive(const DotsTransportHeader& transportHeader, Transmission&& transmission)
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
            LOG_ERROR_S("exception in receive: " << e.what());

            return false;
        }
	}
	
	void Transceiver::handleControlMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission)
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
                // No break here: falltrough
                // process all messages, put non-cache messages into buffer
                [[fallthrough]];
            case DotsConnectionState::connected:
                {
                    if (transmission.instance()->_is<DotsCacheInfo>()) 
                    {
                        // TODO: implement handling of DotsCacheInfo
                        // for now let trough like an normal object
                    }

                    DotsHeader dotsHeader = transportHeader.dotsHeader;
                    dotsHeader.isFromMyself(dotsHeader.sender == m_id);
                    m_dispatcher.dispatch(dotsHeader, transmission.instance());
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
	
	void Transceiver::handleRegularMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission)
	{
		switch(m_connectionState)
        {
            case DotsConnectionState::connecting:
                break;
            case DotsConnectionState::early_subscribe:
            case DotsConnectionState::connected:
                {
                    DotsHeader dotsHeader = transportHeader.dotsHeader;
                    dotsHeader.isFromMyself(dotsHeader.sender == m_id);
                    m_dispatcher.dispatch(dotsHeader, transmission.instance());
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
			onEarlySubscribe();
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
}