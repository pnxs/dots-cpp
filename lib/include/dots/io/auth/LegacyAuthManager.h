#pragma once
#include <string>
#include <map>
#include <dots/asio.h>
#include <dots/io/auth/AuthManager.h>
#include <dots/Subscription.h>
#include <DotsMsgConnect.dots.h>
#include <DotsAuthentication.dots.h>
#include <DotsAuthenticationPolicy.dots.h>

namespace dots::io
{
    struct LegacyAuthManager : AuthManager
    {
        using rules_t = std::multimap<uint16_t, DotsAuthentication>;

        LegacyAuthManager(HostTransceiver& transceiver);
        LegacyAuthManager(const LegacyAuthManager& other) = delete;
        LegacyAuthManager(LegacyAuthManager&& other) = default;
        ~LegacyAuthManager() override = default;

        LegacyAuthManager& operator = (const LegacyAuthManager& rhs) = delete;
        LegacyAuthManager& operator = (LegacyAuthManager&& rhs) = default;

        std::optional<Nonce> requiresAuthentication(const Endpoint& remoteEndpoint, std::string_view guest) override;
        bool verifyAuthentication(const Endpoint& remoteEndpoint, std::string_view guest, Nonce nonce, Nonce cnonce, const Digest& response) override;
        bool verifyAuthentication(const Endpoint& remoteEndpoint, std::string_view guest, Nonce nonce, std::string_view cnonce, const Digest& response) override;

        const rules_t& rules() const;
        const std::optional<bool>& defaultPolicy() const;

        bool requiresAuthentication(const asio::ip::address& address);
        bool verifyResponse(const asio::ip::address& address, uint64_t authNonce, const DotsMsgConnect& msgConnect);

        std::vector<DotsAuthentication> findMatchingRules(const asio::ip::address& address, const std::string& clientName);

    private:

        void handleDotsAuthentication(const Event<DotsAuthentication>& e);
        void handleDotsAuthenticationPolicy(const Event<DotsAuthenticationPolicy>& e);

        Subscription m_dotsAuthenticationSubscription;
        Subscription m_dotsAuthenticationPolicySubscription;
        rules_t m_rules;
        std::optional<bool> m_defaultAcceptPolicy;
    };
}
