#include <dots/type/EnumDescriptor.h>
#include "Connection.h"
#include "ConnectionManager.h"
#include "dots/io/Registry.h"
#include <dots/io/DescriptorConverter.h>

#include "DotsMsgConnectResponse.dots.h"
#include "DotsMsgError.dots.h"
#include "DotsMsgHello.dots.h"
#include "DotsClient.dots.h"
#include "DotsCacheInfo.dots.h"

#include "EnumDescriptorData.dots.h"
#include "StructDescriptorData.dots.h"

#include <dots/dots.h>

#define CEXPANSION "Connection[" << name() << "," << id() << "]: "
#define PEXPSTR_PRE "Connection[%p,%s]: "
#define PEXPARGS_PRE ,this, name().c_str(),
#include "dots/common/ext_logging.h"

namespace dots
{
    using namespace std::placeholders;

    Connection::Connection(channel_ptr_t channel) :
        m_connectionState(DotsConnectionState::closed),
        m_channel(std::move(channel)),
        m_name("<not_set>"),
        m_id(M_nextClientId++)
    {
        /* do nothing */
    }

    DotsConnectionState Connection::state() const
    {
        return m_connectionState;
    }

    const Connection::id_t& Connection::id() const
    {
        return m_id;
    }

    const string& Connection::name() const
    {
        return m_name;
    }

    void Connection::asyncReceive(io::Registry& registry, const std::string& serverName, receive_handler_t&& receiveHandler, error_handler_t&& errorHandler)
	{
		if (m_connectionState != DotsConnectionState::closed)
        {
            throw std::logic_error{ "only one async receive can be active at the same time" };
        }

		m_registry = &registry;
		m_receiveHandler = std::move(receiveHandler);
		m_errorHandler = std::move(errorHandler);
		
		m_connectionState = DotsConnectionState::connecting;
		m_channel->asyncReceive(registry,
			[this](const DotsTransportHeader& transportHeader, Transmission&& transmission){ return handleReceive(transportHeader, std::move(transmission)); },
			[this](const std::exception& e){ handleError(e); }
		);

        transmit(DotsMsgHello{
            DotsMsgHello::serverName_i{ serverName },
            DotsMsgHello::authChallenge_i{ 0 }
        });
	}

    void Connection::transmit(const type::Struct& instance, types::property_set_t includedProperties/* = types::property_set_t::All*/, bool remove/* = false*/)
    {
        const type::StructDescriptor<>& descriptor = instance._descriptor();

        DotsTransportHeader header{
            DotsTransportHeader::destinationGroup_i{ descriptor.name() },
            DotsTransportHeader::dotsHeader_i{
                DotsHeader::typeName_i{ descriptor.name() },
                DotsHeader::sentTime_i{ pnxs::SystemNow() },
                DotsHeader::attributes_i{ includedProperties ==  types::property_set_t::All ? instance._validProperties() : includedProperties },
                DotsHeader::sender_i{ ServerId },
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

    /**
     * Central method to process received messages
     * @param msg
     * @param data
     *
     * @code
     * Table of accepted messages
     *
     * State      | Dots    | Dots   | Other |
     *            | Connect | Member |       |
     * -----------+---------+--------+-------+
     * Connecting |   X                      |
     * EarlySub   |             X            |
     * Connected  |             X        X   |
     * Suspended  |                          |
     * Closed     |                          |
     * -----------+--------------------------+
     * @endcode
     */
    bool Connection::handleReceive(const DotsTransportHeader& transportHeader, Transmission&& transmission)
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
                handled = handleControlMessage(modifiedHeader, std::move(transmission));
            }
            else
            {
                handled = handleRegularMessage(modifiedHeader, std::move(transmission));
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
    bool Connection::handleControlMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission)
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

    bool Connection::handleRegularMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission)
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

    void Connection::handleError(const std::exception& e)
    {
        LOG_ERROR_S("channel error in async receive: " << e.what());
        error_handler_t errorHandler;
        errorHandler.swap(m_errorHandler);

        m_registry = nullptr;
		m_receiveHandler = nullptr;
		setConnectionState(DotsConnectionState::closed);

        errorHandler(m_id, e);
    }

    void Connection::setConnectionState(const DotsConnectionState& state)
    {
        LOG_DEBUG_S("change connection state to " << state);
        m_connectionState = state;

        DotsClient{ DotsClient::id_i{ id() }, DotsClient::connectionState_i{ state } }._publish();
    }

    void Connection::importType(const type::Struct& instance)
    {
        if (auto* structDescriptorData = instance._as<StructDescriptorData>())
        {
        	if (bool isNewSharedType = m_sharedTypes.emplace(structDescriptorData->name).second; isNewSharedType)
        	{
        		io::DescriptorConverter{ *m_registry }(*structDescriptorData);
        	}
        }
        else if (auto* enumDescriptorData = instance._as<EnumDescriptorData>())
        {
        	if (bool isNewSharedType = m_sharedTypes.emplace(enumDescriptorData->name).second; isNewSharedType)
        	{
        		io::DescriptorConverter{ *m_registry }(*enumDescriptorData);
        	}
        }
    }
}
