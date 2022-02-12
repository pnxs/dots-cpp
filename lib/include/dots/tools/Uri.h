#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace dots::tools
{
    struct Uri
    {
        Uri(std::string uriStr);
        Uri(const std::string& scheme, const std::string& host, const std::string& port);
        Uri(const std::string& scheme, const std::string& host, uint16_t port);
        Uri(const std::string& scheme, const std::string& path);
        Uri(const Uri& other);
        Uri(Uri&& other);
        ~Uri() = default;

        Uri& operator = (const Uri& rhs);
        Uri& operator = (Uri&& rhs);

        std::string_view uriStr() const;

        std::string_view scheme() const;
        void setScheme(const std::string& scheme);

        std::string_view authority() const;
        void setAuthority(const std::string& authority);

        std::string_view userName() const;
        void setUserName(const std::string& userName);

        std::string_view userPassword() const;
        void setUserPassword(const std::string& userPassword);

        std::string_view host() const;
        void setHost(const std::string& host);

        std::string_view port() const;
        void setPort(const std::string& port);

        std::string_view path() const;
        void setPath(const std::string& path);

        static std::vector<Uri> FromStrings(const std::string& uriStrs);

    private:

        void replace(std::string_view part, std::string_view replacement);
        void parse();

        std::string m_uriStr;

        std::string_view m_scheme;

        std::string_view m_authority;
        std::string_view m_userName;
        std::string_view m_userPassword;
        std::string_view m_hostDelimited;
        std::string_view m_host;
        std::string_view m_port;

        std::string_view m_path;
    };
}
