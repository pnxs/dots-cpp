#include <dots/io/auth/LegacyAuthManager.h>
#include <dots/HostTransceiver.h>
#include <dots/io/auth/Digest.h>
#include <dots/tools/IpNetwork.h>
#include <dots/tools/logging.h>

namespace dots::io
{
    LegacyAuthManager::LegacyAuthManager(HostTransceiver& transceiver) :
        AuthManager(transceiver),
        m_dotsAuthenticationSubscription{ transceiver.subscribe<DotsAuthentication>([this](const Event<DotsAuthentication>& e){ handleDotsAuthentication(e); }) },
        m_dotsAuthenticationPolicySubscription{ transceiver.subscribe<DotsAuthenticationPolicy>([this](const Event<DotsAuthenticationPolicy>& e){ handleDotsAuthenticationPolicy(e); }) }
    {
        /* do nothing */
    }

    std::optional<Nonce> LegacyAuthManager::requiresAuthentication(const Endpoint& remoteEndpoint, std::string_view/* guest*/)
    {
        if (remoteEndpoint.scheme() == "tcp" || remoteEndpoint.scheme() == "ws")
        {
            if (requiresAuthentication(asio::ip::address::from_string(std::string{ remoteEndpoint.host() })))
            {
                return Nonce{};
            }
            else
            {
                return std::nullopt;
            }
        }
        else if (remoteEndpoint.scheme() == "uds")
        {
            return std::nullopt;
        }
        else
        {
            if (m_defaultAcceptPolicy == true)
            {
                return std::nullopt;
            }
            else
            {
                return Nonce{};
            }
        }
    }

    bool LegacyAuthManager::verifyAuthentication(const Endpoint& remoteEndpoint, std::string_view guest, Nonce nonce, Nonce cnonce, const Digest& response)
    {
        return verifyAuthentication(remoteEndpoint, guest, nonce, cnonce.toString(), response);
    }

    bool LegacyAuthManager::verifyAuthentication(const Endpoint& remoteEndpoint, std::string_view guest, Nonce nonce, std::string_view cnonce, const Digest& response)
    {
        if (remoteEndpoint.scheme() == "tcp" || remoteEndpoint.scheme() == "ws")
        {
            DotsMsgConnect connect{
                DotsMsgConnect::clientName_i{ guest },
                DotsMsgConnect::authChallengeResponse_i{ response.toString() },
                DotsMsgConnect::cnonce_i{ cnonce }
            };

            return verifyResponse(asio::ip::address::from_string(std::string{ remoteEndpoint.host() }), nonce.value(), connect);
        }
        else if (remoteEndpoint.scheme() == "uds")
        {
            return true;
        }
        else
        {
            return m_defaultAcceptPolicy.value_or(false);
        }
    }

    auto LegacyAuthManager::rules() const -> const rules_t&
    {
        return m_rules;
    }

    const std::optional<bool>& LegacyAuthManager::defaultPolicy() const
    {
        return m_defaultAcceptPolicy;
    }

    bool LegacyAuthManager::verifyResponse(const asio::ip::address& address, uint64_t authNonce, const DotsMsgConnect& msgConnect)
    {
        std::optional<bool> accept;

        if (auto rules = findMatchingRules(address, *msgConnect.clientName); !rules.empty())
        {
            for (const DotsAuthentication& rule : rules)
            {
                if (rule.secret.isValid())
                {
                    if (!msgConnect._hasProperties(DotsMsgConnect::authChallengeResponse_p + DotsMsgConnect::cnonce_p))
                    {
                        accept = false;
                        break;
                    }

                    Digest response{ *msgConnect.authChallengeResponse };
                    Digest expected{ Nonce{ authNonce }, *msgConnect.cnonce, *msgConnect.clientName, *rule.secret };

                    if (response.value() == expected.value())
                    {
                        if (rule.accept.isValid())
                        {
                            accept = *rule.accept;
                            break;
                        }
                    }
                }
                else
                {
                    if (rule.accept.isValid())
                    {
                        accept = *rule.accept;
                        break;
                    }
                }
            }
        }

        if (!accept.has_value())
        {
            accept = m_defaultAcceptPolicy;
        }

        return accept.value_or(true);
    }

    bool LegacyAuthManager::requiresAuthentication(const asio::ip::address& address)
    {
        for (const DotsAuthentication& authentication : findMatchingRules(address, {}))
        {
            if (authentication.accept == true)
            {
                return authentication.secret.isValid();
            }
        }

        return false;
    }

    std::vector<DotsAuthentication> LegacyAuthManager::findMatchingRules(const asio::ip::address& address, const std::string& clientName)
    {
        tools::IpNetwork network{ address };

        std::vector<DotsAuthentication> matchtingRules;

        for (auto& iter : m_rules)
        {
            try
            {
                DotsAuthentication& rule = iter.second;

                if ((rule.clientName->empty() || *rule.clientName == clientName) && network.isSubnetOf(tools::IpNetwork{ *rule.network->network, *rule.network->prefix }))
                {
                    LOG_DEBUG_S("Match: " << *rule.network->network << "/" << +*rule.network->prefix << " (" << (rule.secret.isValid() ? "with secret" : "without secret") << ", accept=" << *rule.accept << ")");
                    matchtingRules.emplace_back(rule);
                }
            }
            catch (const std::exception& e)
            {
                LOG_WARN_S("Error processing DotsAuthentication obj: " << e.what());
            }
        }

        return matchtingRules;
    }

    void LegacyAuthManager::handleDotsAuthentication(const Event<DotsAuthentication>& e)
    {
        const DotsAuthentication& authentication = e();

        auto key_equal = [&](const auto& authenticationKv)
        {
            return authenticationKv.second._equal(authentication, DotsAuthentication::_KeyProperties());
        };

        if (e.isCreate())
        {
            m_rules.emplace(authentication.priority.valueOrDefault(0), authentication);
        }
        else if (e.isUpdate())
        {
            if (auto it = std::find_if(m_rules.begin(), m_rules.end(), key_equal); it != m_rules.end())
            {
                it->second._copy(authentication);
            }
        }
        else if (e.isRemove())
        {
            if (auto it = std::find_if(m_rules.begin(), m_rules.end(), key_equal); it != m_rules.end())
            {
                m_rules.erase(it);
            }
        }
    }

    void LegacyAuthManager::handleDotsAuthenticationPolicy(const Event<DotsAuthenticationPolicy>& e)
    {
        if (const DotsAuthenticationPolicy& policy = e(); e.isRemove())
        {
            m_defaultAcceptPolicy.reset();
        }
        else
        {
            if (policy.accept.isValid())
            {
                m_defaultAcceptPolicy = *policy.accept;
            }
        }
    }
}
