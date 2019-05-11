#include "VirtualChannel.h"
#include <dots/common/Chrono.h>
#include <DotsMsgError.dots.h>
#include <DotsMsgHello.dots.h>
#include <DotsMsgConnect.dots.h>
#include <DotsMsgConnectResponse.dots.h>
#include <DotsMember.dots.h>
#include <DotsClearCache.dots.h>

namespace dots
{
	VirtualChannel::VirtualChannel(asio::io_context& ioContext, std::string serverName/* = "VirtualChannel"*/, bool skipHandshake/* = false*/) :
		m_ioContext{ std::ref(ioContext) },
        m_serverName{ std::move(serverName) }
	{
        if (skipHandshake)
        {
            m_connectionState = DotsConnectionState::connected;
            asio::post(m_ioContext.get(), [this](){ onConnected(); });
        }
        else
        {
            m_connectionState = DotsConnectionState::connecting;
        }
	}

	void VirtualChannel::asyncReceive(receive_handler_t&& receiveHandler, error_handler_t&&/* errorHandler*/)
	{
		m_receiveHandler = std::move(receiveHandler);

        if (m_connectionState == DotsConnectionState::connecting)
        {
            spoof(ServerId, DotsMsgHello{
                DotsMsgHello::serverName_t_i{ m_serverName },
                DotsMsgHello::authChallenge_t_i{ 0 }
            });
        }        
	}

	void VirtualChannel::transmit(const DotsTransportHeader& header, const type::Struct& instance)
	{
        if (header.nameSpace == "SYS")
        {
            switch(m_connectionState)
            {
                case DotsConnectionState::connecting:
                    if (auto* dotsMsgConnect = instance._as<DotsMsgConnect>())
                    {
                        DotsMsgConnectResponse dotsMsgConnectResponse{
                            DotsMsgConnectResponse::serverName_t_i{ m_serverName },
                            DotsMsgConnectResponse::clientId_t_i{ ClientId },
                            DotsMsgConnectResponse::accepted_t_i{ true },
                        };

                        if (dotsMsgConnect->preloadCache == true)
                        {
                            dotsMsgConnectResponse.preload(true);
                            m_connectionState = DotsConnectionState::early_subscribe;
                            spoof(ServerId, dotsMsgConnectResponse);
                        }
                        else
                        {
                            m_connectionState = DotsConnectionState::connected;
                            spoof(ServerId, dotsMsgConnectResponse);
                            asio::post(m_ioContext.get(), [this](){ onConnected(); });
                        }
                    }
                    break;
                case DotsConnectionState::early_subscribe:
                    if (instance._is<DotsMsgConnect>())
                    {
                        m_connectionState = DotsConnectionState::connected;
                        spoof(ServerId, DotsMsgConnectResponse{ DotsMsgConnectResponse::preloadFinished_t_i{ true } });
                        asio::post(m_ioContext.get(), [this](){ onConnected(); });
                    }
                    [[fallthrough]];
                case DotsConnectionState::connected:
                    if (auto* dotsMember = instance._as<DotsMember>())
                    {
                        if (dotsMember->_hasProperties(DotsMember::groupName_t::Set() + DotsMember::event_t::Set()))
                        {
                            if (dotsMember->event == DotsMemberEvent::join)
                            {
                                m_subscribedTypes.insert(dotsMember->groupName);
                                onSubscribe(dotsMember->groupName);
                            }
                            else if (dotsMember->event == DotsMemberEvent::leave)
                            {
                                m_subscribedTypes.erase(dotsMember->groupName);
                                onUnsubscribe(dotsMember->groupName);
                            }
                        }
                        
                    }
                    else if (instance._is<DotsClearCache>())
                    {
                        /* not yet supported */
                    }
                    break;
                default:
                    /* do nothing */
                    break;
            }
        }
        else if (m_connectionState == DotsConnectionState::connected)
        {
            onTransmit(header, instance);

            if (m_subscribedTypes.count(header.dotsHeader->typeName))
            {
                spoof(header, instance);
            }
        }		
	}

    void VirtualChannel::spoof(const DotsTransportHeader& header, const type::Struct& instance)
    {
        asio::post(m_ioContext.get(), [this, _header = header, _instance = type::AnyStruct{ instance }]() mutable
        {
            _header.dotsHeader->sender.constructOrValue(ClientId);
            _header.dotsHeader->serverSentTime(pnxs::SystemNow{});
            m_receiveHandler(_header, Transmission{ std::move(_instance) });
        });
    }
    
    void VirtualChannel::spoof(uint32_t sender, const type::Struct& instance, bool remove/* = false*/)
    {
        const type::StructDescriptor& descriptor = instance._descriptor();

        DotsTransportHeader transportHeader{
            DotsTransportHeader::destinationGroup_t_i{ descriptor.name() },
            DotsTransportHeader::dotsHeader_t_i{
                DotsHeader::typeName_t_i{ descriptor.name() },
                DotsHeader::sentTime_t_i{ pnxs::SystemNow() },
                DotsHeader::attributes_t_i{ instance._validProperties() },
                DotsHeader::sender_t_i{ sender },
                DotsHeader::removeObj_t_i{ remove },
            }
        };

        if (descriptor.internal())
        {
            transportHeader.nameSpace("SYS");
        }

        spoof(transportHeader, instance);
    }

    void VirtualChannel::onConnected()
    {
        /* do nothing */
    }

    void VirtualChannel::onSubscribe(const std::string& name)
    {
        /* do nothing */
    }

    void VirtualChannel::onUnsubscribe(const std::string& name)
    {
        /* do nothing */
    }

    void VirtualChannel::onTransmit(const DotsTransportHeader& header, const type::Struct& instance)
    {
        /* do nothing */
    }

    const std::set<std::string>& VirtualChannel::subscribedTypes() const
    {
        return m_subscribedTypes;
    }
}