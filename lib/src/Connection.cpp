#include <dots/Connection.h>
#include <dots/type/Registry.h>
#include <dots/io/auth/Digest.h>
#include <dots/tools/logging.h>
#include <dots/serialization/StringSerializer.h>
#include <DotsMsgConnect.dots.h>
#include <DotsClient.dots.h>
#include <DotsDescriptorRequest.dots.h>

// suppress deprecation warning for using legacy authentication interface
#if (defined _MSC_VER)
#pragma warning(disable: 4996)
#elif (defined __GNUG__)
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#define LOG_TRANSMISSION(prefix_, header_, instance_)                                                                                                               \
{                                                                                                                                                                   \
    if ((instance_)._descriptor().internal())                                                                                                                       \
    {                                                                                                                                                               \
        LOG_DATA_S(prefix_ << peerDescription() << "\n" << dots::to_string(header_, StringOptions) << ",\n" << dots::to_string(instance_, StringOptions) << "\n");  \
    }                                                                                                                                                               \
    else                                                                                                                                                            \
    {                                                                                                                                                               \
        LOG_DEBUG_S(prefix_ << peerDescription() << "\n" << dots::to_string(header_, StringOptions) << ",\n" << dots::to_string(instance_, StringOptions) << "\n"); \
    }                                                                                                                                                               \
}

#define LOG_TRANSMIT_TRANSMISSION(header_, instance_) LOG_TRANSMISSION("TRANSMIT to ", header_, instance_)
#define LOG_RECEIVE_TRANSMISSION(header_, instance_) LOG_TRANSMISSION("RECEIVE from ", header_, instance_)

namespace dots
{
    Connection::Connection(io::channel_ptr_t channel, bool host, std::optional<std::string> authSecret/* = std::nullopt*/) :
        m_expectedSystemType{ &DotsMsgError::_Descriptor(), types::property_set_t::None, nullptr },
        m_connectionState(DotsConnectionState::suspended),
        m_selfId(host ? HostId : UninitializedId),
        m_peerId(host ? M_nextGuestId++ : HostId),
        m_peerName("<not_set>"),
        m_channel(std::move(channel)),
        m_authSecret{ std::move(authSecret) },
        m_registry(nullptr)
    {
        /* do nothing */
    }

    Connection::~Connection() noexcept
    {
        if (m_connectionState == DotsConnectionState::connected)
        {
            try
            {
                transmit(DotsMsgError{ DotsMsgError::errorCode_i{ 0 } });
            }
            catch (const std::exception&/* e*/)
            {
                /* do nothing */
            }
        }
    }

    const io::Endpoint& Connection::localEndpoint() const
    {
        return m_channel->localEndpoint();
    }

    const io::Endpoint& Connection::remoteEndpoint() const
    {
        return m_channel->remoteEndpoint();
    }

    DotsConnectionState Connection::state() const
    {
        return m_connectionState;
    }

    auto Connection::selfId() const -> id_t
    {
        return m_selfId;
    }

    auto Connection::peerId() const -> id_t
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

    bool Connection::closed() const
    {
        return m_connectionState == DotsConnectionState::closed;
    }

    std::string Connection::peerDescription() const
    {
        return (m_selfId == HostId ? "guest '" : "host '") + m_peerName + " [" + std::to_string(m_peerId) + "]'";
    }

    std::string Connection::endpointDescription() const
    {
        return "from '" + std::string{ m_channel->localEndpoint().uriStr() } + "' at '" + std::string{ m_channel->remoteEndpoint().uriStr() } + "'";
    }

    void Connection::asyncReceive(type::Registry& registry, io::AuthManager* authManager, const std::string_view& name, receive_handler_t&& receiveHandler, transition_handler_t&& transitionHandler)
    {
        if (m_connectionState != DotsConnectionState::suspended)
        {
            throw std::logic_error{ "only one async receive can be started on a connection" };
        }

        if (receiveHandler == nullptr || transitionHandler == nullptr)
        {
            throw std::logic_error{ "both a receive and a transition handler must be set" };
        }

        m_registry = &registry;
        m_authManager = authManager;
        m_receiveHandler = std::move(receiveHandler);
        m_transitionHandler = std::move(transitionHandler);

        setConnectionState(DotsConnectionState::connecting);
        m_channel->init(*m_registry);
        m_channel->asyncReceive(
            [this](io::Transmission transmission){ return handleReceive(std::move(transmission)); },
            [this](const std::exception_ptr& e){ handleError(e); }
        );

        if (m_selfId == HostId)
        {
            if (m_nonce = m_authManager == nullptr ? std::nullopt : m_authManager->requiresAuthentication(m_channel->remoteEndpoint(), {}); m_nonce == std::nullopt)
            {
                transmit(DotsMsgHello{
                    DotsMsgHello::serverName_i{ name },
                    DotsMsgHello::authChallenge_i{ 0 }
                });


            }
            else
            {
                transmit(DotsMsgHello{
                    DotsMsgHello::serverName_i{ name },
                    DotsMsgHello::authChallenge_i{ m_nonce->value() },
                    DotsMsgHello::authenticationRequired_i{ true }
                });
            }

            expectSystemType<DotsMsgConnect>(DotsMsgConnect::clientName_p + DotsMsgConnect::preloadCache_p, &Connection::handleConnect);
        }
        else
        {
            m_selfName = name;
            expectSystemType<DotsMsgHello>(DotsMsgHello::serverName_p + DotsMsgHello::authChallenge_p, &Connection::handleHello);
        }
    }

    void Connection::transmit(const type::Struct& instance, std::optional<types::property_set_t> includedProperties/* = std::nullopt*/, bool remove/* = false*/)
    {
        if (includedProperties == std::nullopt)
        {
            includedProperties = instance._validProperties();
        }
        else
        {
            *includedProperties ^= instance._descriptor().properties();
        }

        transmit(DotsHeader{
            DotsHeader::typeName_i{ instance._descriptor().name() },
            DotsHeader::sentTime_i{ types::timepoint_t::Now() },
            DotsHeader::attributes_i{ *includedProperties },
            DotsHeader::removeObj_i{ remove }
        }, instance);
    }

    void Connection::transmit(const DotsHeader& header, const type::Struct& instance)
    {
        LOG_TRANSMIT_TRANSMISSION(header, instance);
        m_channel->transmit(header, instance);
    }

    void Connection::transmit(const io::Transmission& transmission)
    {
        LOG_TRANSMIT_TRANSMISSION(transmission.header(), *transmission.instance());
        m_channel->transmit(transmission);
    }

    void Connection::transmit(const type::StructDescriptor<>& descriptor)
    {
        m_channel->transmit(descriptor);
    }

    void Connection::handleError(const std::exception_ptr& e)
    {
        if (m_connectionState == DotsConnectionState::connected)
        {
            try
            {
                std::rethrow_exception(e);
            }
            catch (const std::exception& e)
            {
                try
                {
                    transmit(DotsMsgError{
                        DotsMsgError::errorCode_i{ 1 },
                        DotsMsgError::errorText_i{ e.what() }
                    });
                }
                catch (...)
                {
                    /* do nothing */
                }

            }
        }

        handleClose(e);
    }

    bool Connection::handleReceive(io::Transmission transmission)
    {
        LOG_RECEIVE_TRANSMISSION(transmission.header(), *transmission.instance());

        if (m_connectionState == DotsConnectionState::closed)
        {
            return false;
        }

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
                return true;
            }
        }
        else
        {
            if (m_connectionState == DotsConnectionState::connected || m_connectionState == DotsConnectionState::early_subscribe)
            {
                DotsHeader& header = transmission.header();

                if (m_selfId == HostId)
                {
                    header.sender = m_peerId;

                    header.serverSentTime = types::timepoint_t::Now();

                    if (!header.sentTime.isValid())
                    {
                        header.sentTime = header.serverSentTime;
                    }

                    header.isFromMyself = header.sender == m_selfId;
                    return m_receiveHandler(*this, std::move(transmission));
                }
                else
                {
                    if (!header.sentTime.isValid())
                    {
                        header.sentTime(types::timepoint_t::Now());
                    }

                    if (header.sender.isValid())
                    {
                        header.isFromMyself = header.sender == m_selfId;
                        return m_receiveHandler(*this, std::move(transmission));
                    }
                    else
                    {
                        header.sender(m_peerId);
                        header.isFromMyself = false;
                        return m_receiveHandler(*this, std::move(transmission));
                    }
                }
            }
            else
            {
                throw std::logic_error{ "received instance of non-system type " + instance._descriptor().name() + " while not in early_subscribe or connected state " + to_string(m_connectionState) };
            }
        }
    }

    void Connection::handleClose(const std::exception_ptr& e)
    {
        m_receiveHandler = nullptr;
        m_registry = nullptr;
        expectSystemType<DotsMsgError>(types::property_set_t::None, nullptr);
        setConnectionState(DotsConnectionState::closed, e);
    }

    void Connection::handleHello(const DotsMsgHello& hello)
    {
        m_peerName = hello.serverName;

        DotsMsgConnect connect{
            DotsMsgConnect::clientName_i{ m_selfName },
            DotsMsgConnect::preloadCache_i{ true }
        };

        if (hello.authenticationRequired == true)
        {
            if (m_authSecret == std::nullopt)
            {
                throw std::runtime_error{ "host requested authentication but no secret was specified" };
            }

            connect.cnonce = io::Nonce{}.toString();
            connect.authChallengeResponse = io::Digest{ *hello.authChallenge, *connect.cnonce, m_selfName, *m_authSecret }.toString();
        }

        transmit(connect);

        expectSystemType<DotsMsgConnectResponse>(DotsMsgConnectResponse::clientId_p + DotsMsgConnectResponse::accepted_p + DotsMsgConnectResponse::preload_p, &Connection::handleAuthorizationRequest);
    }

    void Connection::handleAuthorizationRequest(const DotsMsgConnectResponse& connectResponse)
    {
        m_selfId = connectResponse.clientId;

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
        if (m_authManager != nullptr && !m_authManager->verifyAuthentication(m_channel->remoteEndpoint(), *connect.clientName, m_nonce.value_or(0), connect.cnonce.valueOrDefault(""), io::Digest{ connect.authChallengeResponse.valueOrDefault("") }))
        {
            transmit(DotsMsgConnectResponse{
                DotsMsgConnectResponse::clientId_i{ m_peerId },
                DotsMsgConnectResponse::preload_i{ connect.preloadCache == true },
                DotsMsgConnectResponse::accepted_i{ false },
            });

            throw std::runtime_error{ "invalid authorization information" };
        }

        m_peerName = connect.clientName;

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
            handleError(std::make_exception_ptr(std::runtime_error{ what }));
        }
    }

    void Connection::setConnectionState(DotsConnectionState state, const std::exception_ptr& e/* = nullptr*/)
    {
        m_connectionState = state;

        try
        {
            if (m_connectionState == DotsConnectionState::connecting)
            {
                LOG_DEBUG_S(peerDescription() << " is establishing connection " << endpointDescription());
            }
            else if (m_connectionState == DotsConnectionState::early_subscribe)
            {
                LOG_DEBUG_S(peerDescription() << " is preloading " << endpointDescription());
            }
            else if (m_connectionState == DotsConnectionState::connected)
            {
                LOG_NOTICE_S(peerDescription() << " established connection " << endpointDescription());
            }
            else if (m_connectionState == DotsConnectionState::closed)
            {
                if (e == nullptr)
                {
                    LOG_NOTICE_S(peerDescription() << " gracefully closed connection");
                }
                else
                {
                    try
                    {
                        std::rethrow_exception(e);
                    }
                    catch (const std::exception& e)
                    {
                        LOG_ERROR_S(peerDescription() << " closed connection with error -> " << e.what());
                    }
                }
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR_S("error while handling transition for connection " << peerDescription() << " -> " << e.what());
        }

        try
        {
            m_transitionHandler(*this, e);
        }
        catch (const std::exception& e)
        {
            throw std::logic_error{ std::string{ "error in transition handler for connection " + peerDescription() + " -> " } + e.what() };
        }
    }

    template <typename T>
    void Connection::expectSystemType(const types::property_set_t& expectedAttributes, void(Connection::* handler)(const T&))
    {
        m_expectedSystemType = { &T::_Descriptor(), expectedAttributes, [this, handler](const type::Struct& instance){ (this->*handler)(instance._to<T>()); } };
    }
}