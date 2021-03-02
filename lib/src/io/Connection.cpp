#include <dots/io/Connection.h>
#include <dots/io/Registry.h>
#include <dots/io/auth/Digest.h>
#include <dots/tools/logging.h>
#include <dots/io/serialization/StringSerializer.h>
#include <DotsMsgConnect.dots.h>
#include <DotsCacheInfo.dots.h>
#include <DotsClient.dots.h>
#include <DotsDescriptorRequest.dots.h>

namespace dots::io
{
    Connection::Connection(channel_ptr_t channel, bool host, std::optional<std::string> authSecret/* = std::nullopt*/) :
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
            catch (const std::exception& e)
            {
                /* do nothing */
            }
        }
    }

    const Medium& Connection::medium() const
    {
        return m_channel->medium();
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

    void Connection::asyncReceive(Registry& registry, AuthManager* authManager, const std::string_view& name, receive_handler_t&& receiveHandler, transition_handler_t&& transitionHandler)
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
            [this](Transmission transmission){ return handleReceive(std::move(transmission)); },
            [this](const std::exception_ptr& e){ handleError(e); }
        );

        if (m_selfId == HostId)
        {
            if (m_nonce = m_authManager == nullptr ? std::nullopt : m_authManager->requiresAuthentication(m_channel->medium(), {}); m_nonce == std::nullopt)
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
        m_channel->transmit(header, instance);
    }

    void Connection::transmit(const Transmission& transmission)
    {
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

    bool Connection::handleReceive(Transmission transmission)
    {
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
                    m_receiveHandler(*this, std::move(transmission));
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
                        m_receiveHandler(*this, std::move(transmission));
                    }
                    else
                    {
                        header.sender(m_peerId);
                        header.isFromMyself = false;
                        m_receiveHandler(*this, std::move(transmission));
                    }
                }
            }
            else
            {
                throw std::logic_error{ "received instance of non-system type " + instance._descriptor().name() + " while not in early_subscribe or connected state " + to_string(m_connectionState) };
            }
        }

        return true;
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
        LOG_DEBUG_S("received hello from '" << *hello.serverName << "' authChallenge=" << hello.authChallenge);

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

            connect.cnonce = Nonce{}.toString();
            connect.authChallengeResponse = Digest{ *hello.authChallenge, *connect.cnonce, m_selfName, *m_authSecret }.toString();
        }

        transmit(connect);

        expectSystemType<DotsMsgConnectResponse>(DotsMsgConnectResponse::clientId_p + DotsMsgConnectResponse::accepted_p + DotsMsgConnectResponse::preload_p, &Connection::handleAuthorizationRequest);
    }

    void Connection::handleAuthorizationRequest(const DotsMsgConnectResponse& connectResponse)
    {
        m_selfId = connectResponse.clientId;
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
        if (m_authManager != nullptr && !m_authManager->verifyAuthentication(m_channel->medium(), *connect.clientName, m_nonce.value_or(0), connect.cnonce.valueOrDefault(""), Digest{ connect.authChallengeResponse.valueOrDefault("") }))
        {
            transmit(DotsMsgConnectResponse{
                DotsMsgConnectResponse::clientId_i{ m_peerId },
                DotsMsgConnectResponse::preload_i{ connect.preloadCache == true },
                DotsMsgConnectResponse::accepted_i{ false },
            });

            throw std::runtime_error{ "invalid authorization information" };
        }

        LOG_INFO_S("authorized");

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
        LOG_DEBUG_S("change connection state to " << to_string(state));
        m_connectionState = state;

        try
        {
            m_transitionHandler(*this, e);
        }
        catch (const std::exception& e)
        {
            throw std::logic_error{ std::string{ "exception in connection transition handler -> " } + e.what() };
        }
    }

    template <typename T>
    void Connection::expectSystemType(const types::property_set_t& expectedAttributes, void(Connection::* handler)(const T&))
    {
        m_expectedSystemType = { &T::_Descriptor(), expectedAttributes, [this, handler](const type::Struct& instance){ (this->*handler)(instance._to<T>()); } };
    }
}