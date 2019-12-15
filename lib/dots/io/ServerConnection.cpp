#include "ServerConnection.h"

#include "DotsTransportHeader.dots.h"
#include "DotsMsgConnect.dots.h"
#include "DotsCacheInfo.dots.h"
#include "dots/io/serialization/AsciiSerialization.h"
#include "Transmitter.h"

namespace dots
{
	bool ServerConnection::start(const string& name, channel_ptr_t channel)
	{
		if (running())
		{
			LOG_WARN_S("already started");
			return true;
		}

		DotsMsgHello::_Descriptor();
		DotsMsgConnectResponse::_Descriptor();
		DotsCacheInfo::_Descriptor();

		m_channel = channel;
		m_channel->asyncReceive(FUN(*this, handleReceivedMessage), nullptr);

		m_running = true;
		m_clientName = name;

		handleConnected(name);

		return true;
	}

	void ServerConnection::stop()
	{
		if (!running()) return;

		disconnect();

		m_running = false;
	}

	bool ServerConnection::running()
	{
		return m_running;
	}

	void ServerConnection::disconnect()
	{
		m_channel.reset();
	}

	Channel& ServerConnection::channel()
	{
		return *m_channel.get();
	}

	void ServerConnection::handleConnected(const string&/*name*/)
	{
		switch (m_connectionState)
		{
			case DotsConnectionState::connecting:
				break;
			case DotsConnectionState::suspended:
				// When a connection is reestablished, continue operation
				break;
			case DotsConnectionState::early_subscribe:
			case DotsConnectionState::connected:
			case DotsConnectionState::closed:
				// Do nothing
				break;
		}
	}

	void ServerConnection::handleDisconnected()
	{
		switch (m_connectionState)
		{
			case DotsConnectionState::connecting:
				// Connect failed
				setConnectionState(DotsConnectionState::closed);
				break;
			case DotsConnectionState::early_subscribe:
				// Connect failed
				setConnectionState(DotsConnectionState::closed);
				break;
			case DotsConnectionState::connected:
				// Connection interrupted, try to reestablish
				setConnectionState(DotsConnectionState::suspended);
				break;
			case DotsConnectionState::suspended:
			case DotsConnectionState::closed:
				// Do nothing
				break;
		}
	}

	bool ServerConnection::handleReceivedMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission)
	{
		try
		{
			if (transportHeader.nameSpace.isValid() && transportHeader.nameSpace == "SYS")
			{
				onControlMessage(transportHeader, std::move(transmission));
			}
			else
			{
				onRegularMessage(transportHeader, std::move(transmission));
			}

			return true;
		}
		catch (const std::exception& e)
		{
			LOG_ERROR_S("exception in receive: " << e.what());
			stop();

			return false;
		}
	}

	void ServerConnection::onControlMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission)
	{
		const auto& typeName = *transportHeader.dotsHeader->typeName;

		switch (m_connectionState)
		{
			case DotsConnectionState::connecting:
				if (typeName == "DotsMsgHello")
				{
					processHello(static_cast<const DotsMsgHello&>(transmission.instance().get()));
				}
				else if (typeName == "DotsMsgConnectResponse")
				{
					processConnectResponse(static_cast<const DotsMsgConnectResponse&>(transmission.instance().get()));
				}
				break;
			case DotsConnectionState::early_subscribe:
				if (typeName == "DotsMsgConnectResponse")
				{
					processEarlySubscribe(static_cast<const DotsMsgConnectResponse&>(transmission.instance().get()));
				}
				// No break here: falltrough
				// process all messages, put non-cache messages into buffer
				[[fallthrough]];
			case DotsConnectionState::connected:
			{
				if (typeName == "DotsCacheInfo")
				{
					//TODO: implement handling of DotsCacheInfo
					//for now let trough like an normal object
					// return;
				}

				DotsHeader dotsHeader = transportHeader.dotsHeader;
				dotsHeader.isFromMyself(dotsHeader.sender == m_serversideClientname);
				onReceiveMessage(dotsHeader, transmission.instance());
			}

			break;
			case DotsConnectionState::suspended:
				// buffer outgoing messages
				break;
			case DotsConnectionState::closed:
				// Do nothing
				break;
		}
	}

	void ServerConnection::onRegularMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission)
	{
		switch (m_connectionState)
		{
			case DotsConnectionState::connecting:
				break;
			case DotsConnectionState::early_subscribe:
			case DotsConnectionState::connected:
			{
				DotsHeader dotsHeader = transportHeader.dotsHeader;
				dotsHeader.isFromMyself(dotsHeader.sender == m_serversideClientname);
				onReceiveMessage(dotsHeader, transmission.instance());
			}
				break;
			case DotsConnectionState::suspended:
				// buffer outgoing messages
				break;
			case DotsConnectionState::closed:
				// Do nothing
				break;
		}
	}

	void ServerConnection::setConnectionState(DotsConnectionState state)
	{
		LOG_DEBUG_S("change connection state to " << to_string(state));
		m_connectionState = state;
		switch (m_connectionState)
		{
			case DotsConnectionState::connected:
				onConnected();
				break;
			case DotsConnectionState::early_subscribe:
				onEarlyConnect();
				break;
			default:
				break;
		}
	}

	void ServerConnection::processConnectResponse(const DotsMsgConnectResponse& connectResponse)
	{
		const std::string& serverName = connectResponse.serverName.isValid() ? *connectResponse.serverName : "<unknown>";
		LOG_DEBUG_S("connectResponse: serverName=" << serverName << " accepted=" << *connectResponse.accepted);
		
		if (connectResponse.clientId.isValid())
		{
			m_serversideClientname = connectResponse.clientId;
		}
		
		if (connectResponse.preload == true && (!connectResponse.preloadFinished.isValid() || connectResponse.preloadFinished == false))
		{
			setConnectionState(DotsConnectionState::early_subscribe);
		}
		else
		{
			setConnectionState(DotsConnectionState::connected);
		}
	}

	void ServerConnection::processEarlySubscribe(const DotsMsgConnectResponse& connectResponse)
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

	void ServerConnection::processHello(const DotsMsgHello& hello)
	{
		if (hello.authChallenge.isValid() && hello.serverName.isValid())
		{
			LOG_DEBUG_S("received hello from '" << *hello.serverName << "' authChallenge=" << hello.authChallenge);
			LOG_DATA_S("send DotsMsgConnect");

			DotsMsgConnect{
                DotsMsgConnect::clientName_i{ m_clientName },
                DotsMsgConnect::preloadCache_i{ true }
            }._publish();
		}
		else
		{
			LOG_WARN_S("Invalid hello from server valatt:" << hello._validProperties().toString());
		}
	}
}