#pragma once
#include <string>
#include <map>
#include <boost/asio.hpp>
#include <dots/io/auth/AuthManager.h>
#include <dots/io/Subscription.h>
#include <DotsMsgConnect.dots.h>
#include <DotsAuthentication.dots.h>
#include <DotsAuthenticationPolicy.dots.h>

namespace dots::io
{
    struct LegacyAuthManager : AuthManager
    {
        using rules_t = std::multimap<uint16_t, DotsAuthentication>;

        LegacyAuthManager(HostTransceiver& transceiver);
        LegacyAuthManager(const LegacyAuthManager& other) = default;
        LegacyAuthManager(LegacyAuthManager&& other) = default;
        ~LegacyAuthManager() = default;

        LegacyAuthManager& operator = (const LegacyAuthManager& rhs) = default;
        LegacyAuthManager& operator = (LegacyAuthManager&& rhs) = default;

        virtual std::optional<Nonce> requiresAuthentication(const Medium& endpoint, std::string_view guest) override;
        virtual bool verifyAuthentication(const Medium& medium, std::string_view guest, Nonce nonce, Nonce cnonce, const Digest& response) override;
        virtual bool verifyAuthentication(const Medium& medium, std::string_view guest, Nonce nonce, std::string_view cnonce, const Digest& response) override;

        const rules_t& rules() const;
        const std::optional<bool>& defaultPolicy() const;

        bool requiresAuthentication(const boost::asio::ip::address& address);
        bool verifyResponse(const boost::asio::ip::address& address, uint64_t authNonce, const DotsMsgConnect& response);

        std::vector<DotsAuthentication> findMatchingRules(const boost::asio::ip::address& address, const std::string& clientName);

    private:

        void handleDotsAuthentication(const Event<DotsAuthentication>& e);
        void handleDotsAuthenticationPolicy(const Event<DotsAuthenticationPolicy>& e);

        bool m_acceptLoopback;
        Subscription m_dotsAuthenticationSubscription;
        Subscription m_dotsAuthenticationPolicySubscription;
        rules_t m_rules;
        std::optional<bool> m_defaultPolicy;
    };
}
