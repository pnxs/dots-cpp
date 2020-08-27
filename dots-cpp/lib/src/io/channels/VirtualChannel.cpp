#include <dots/io/channels/VirtualChannel.h>
#include <dots/type/Chrono.h>
#include <DotsMsgError.dots.h>
#include <DotsMsgHello.dots.h>
#include <DotsMsgConnect.dots.h>
#include <DotsMsgConnectResponse.dots.h>
#include <DotsMember.dots.h>
#include <DotsClearCache.dots.h>

namespace dots::io
{
    VirtualChannel::VirtualChannel(Channel::key_t key, boost::asio::io_context& ioContext, std::string serverName/* = "VirtualChannel"*/, bool skipHandshake/* = false*/) :
        Channel(key),
        m_ioContext{ std::ref(ioContext) },
        m_serverName{ std::move(serverName) }
    {
        if (skipHandshake)
        {
            m_connectionState = DotsConnectionState::connected;
            boost::asio::post(m_ioContext.get(), [this](){ onConnected(); });
        }
        else
        {
            m_connectionState = DotsConnectionState::closed;
        }
    }

    void VirtualChannel::spoof(const DotsHeader& header, const type::Struct& instance)
    {
        boost::asio::post(m_ioContext.get(), [this, _header = header, _instance = type::AnyStruct{ instance }]() mutable
        {
            _header.sender.constructOrValue(ClientId);
            _header.serverSentTime(types::timepoint_t::Now());
            processReceive(Transmission{ _header, std::move(_instance) });
        });
    }

    void VirtualChannel::spoof(uint32_t sender, const type::Struct& instance, bool remove/* = false*/)
    {
        const type::StructDescriptor<>& descriptor = instance._descriptor();

        DotsHeader header{
            DotsHeader::typeName_i{ descriptor.name() },
            DotsHeader::sentTime_i{ types::timepoint_t::Now() },
            DotsHeader::attributes_i{ instance._validProperties() },
            DotsHeader::sender_i{ sender },
            DotsHeader::removeObj_i{ remove }
        };

        spoof(header, instance);
    }

    void VirtualChannel::asyncReceiveImpl()
    {
        if (m_connectionState == DotsConnectionState::closed)
        {
            m_connectionState = DotsConnectionState::connecting;
            spoof(ServerId, DotsMsgHello{
                DotsMsgHello::serverName_i{ m_serverName },
                DotsMsgHello::authChallenge_i{ 0 }
            });
        }
    }

    void VirtualChannel::transmitImpl(const DotsHeader& header, const type::Struct& instance)
    {
        if (instance._descriptor().internal())
        {
            switch(m_connectionState)
            {
                case DotsConnectionState::connecting:
                    if (auto* dotsMsgConnect = instance._as<DotsMsgConnect>())
                    {
                        DotsMsgConnectResponse dotsMsgConnectResponse{
                            DotsMsgConnectResponse::serverName_i{ m_serverName },
                            DotsMsgConnectResponse::clientId_i{ ClientId },
                            DotsMsgConnectResponse::accepted_i{ true },
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
                            boost::asio::post(m_ioContext.get(), [this](){ onConnected(); });
                        }
                    }
                    break;
                case DotsConnectionState::early_subscribe:
                    if (auto* dotsMsgConnect = instance._as<DotsMsgConnect>(); dotsMsgConnect->preloadClientFinished == true)
                    {
                        m_connectionState = DotsConnectionState::connected;
                        spoof(ServerId, DotsMsgConnectResponse{ DotsMsgConnectResponse::preloadFinished_i{ true } });
                        boost::asio::post(m_ioContext.get(), [this](){ onConnected(); });
                    }
                    [[fallthrough]];
                case DotsConnectionState::connected:
                    if (auto* dotsMember = instance._as<DotsMember>())
                    {
                        if (dotsMember->_hasProperties(DotsMember::groupName_p + DotsMember::event_p))
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

            if (m_subscribedTypes.count(header.typeName))
            {
                spoof(header, instance);
            }
        }
    }

    void VirtualChannel::onConnected()
    {
        /* do nothing */
    }

    void VirtualChannel::onSubscribe(const std::string& /*name*/)
    {
        /* do nothing */
    }

    void VirtualChannel::onUnsubscribe(const std::string& /*name*/)
    {
        /* do nothing */
    }

    void VirtualChannel::onTransmit(const DotsHeader& /*name*/, const type::Struct& /*name*/)
    {
        /* do nothing */
    }

    const std::set<std::string>& VirtualChannel::subscribedTypes() const
    {
        return m_subscribedTypes;
    }
}