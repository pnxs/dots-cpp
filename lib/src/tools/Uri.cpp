#include <dots/tools/Uri.h>
#include <dots/tools/string_tools.h>

namespace dots::tools
{
    Uri::Uri(std::string uriStr) :
        m_uriStr{ std::move(uriStr) }
    {
        parse();
    }

    Uri::Uri(const std::string& scheme, const std::string& host, const std::string& port) :
        Uri(scheme + "://" + host + ":" + port)
    {
        /* do nothing */
    }

    Uri::Uri(const std::string& scheme, const std::string& host, uint16_t port) :
        Uri(scheme, host, std::to_string(port))
    {
        /* do nothing */
    }

    Uri::Uri(const std::string& scheme, const std::string& path) :
        Uri(scheme + ":" + path)
    {
        /* do nothing */
    }

    Uri::Uri(const Uri& other) :
        m_uriStr{ other.uriStr() }
    {
        parse();
    }

    Uri::Uri(Uri&& other) :
        m_uriStr{ other.uriStr() }
    {
        parse();
    }

    Uri& Uri::operator = (const Uri& rhs)
    {
        m_uriStr = rhs.m_uriStr;
        parse();

        return *this;
    }

    Uri& Uri::operator = (Uri&& rhs)
    {
        m_uriStr = std::move(rhs.m_uriStr);
        parse();

        return *this;
    }

    std::string_view Uri::uriStr() const
    {
        return m_uriStr;
    }

    std::string_view Uri::scheme() const
    {
        return m_scheme;
    }

    void Uri::setScheme(const std::string& scheme)
    {
        if (scheme.empty())
        {
            throw std::runtime_error{ "URI scheme cannot be empty" };
        }

        replace(m_scheme, scheme);
    }

    std::string_view Uri::authority() const
    {
        return m_authority;
    }

    void Uri::setAuthority(const std::string& authority)
    {
        if (m_authority.empty())
        {
            std::string schemeAndAuthority;
            schemeAndAuthority += m_scheme;
            schemeAndAuthority += "://";
            schemeAndAuthority += authority;
            schemeAndAuthority += m_path;
            replace(m_scheme, schemeAndAuthority);
        }
        else
        {
            replace(m_authority, authority);
        }
    }

    std::string_view Uri::userName() const
    {
        return m_userName;
    }

    void Uri::setUserName(const std::string& userName)
    {
        if (m_userName.empty())
        {
            if (m_host.empty())
            {
                throw std::runtime_error{ "cannot set user name on URI without a host" };
            }

            std::string userAtHost;
            userAtHost += userName;
            userAtHost += '@';
            userAtHost += m_host;
            replace(m_host, userAtHost);
        }
        else
        {
            replace(m_userName, userName);
        }
    }

    std::string_view Uri::userPassword() const
    {
        return m_userPassword;
    }

    void Uri::setUserPassword(const std::string& userPassword)
    {
        if (m_userPassword.empty())
        {
            if (m_userName.empty())
            {
                throw std::runtime_error{ "cannot set user password on URI without a user name" };
            }

            std::string userNameAndPassword;
            userNameAndPassword += m_userName;
            userNameAndPassword += ':';
            userNameAndPassword += userPassword;
            replace(m_userName, userNameAndPassword);
        }
        else
        {
            if (userPassword.empty())
            {
                std::string_view userPasswordAndSeparator{ m_userPassword.data() - 1, m_userPassword.size() + 1 };
                replace(userPasswordAndSeparator, userPassword);
            }
            else
            {
                replace(m_userPassword, userPassword);
            }
        }
    }

    std::string_view Uri::host() const
    {
        return m_host;
    }

    void Uri::setHost(const std::string& host)
    {
        std::string_view hostView = host;
        std::string hostDelimited;

        if (bool isIpv6 = host.find_first_of(':') != std::string_view::npos; isIpv6)
        {
            hostDelimited += '[';
            hostDelimited += host;
            hostDelimited += ']';
            hostView = hostDelimited;
        }

        if (m_authority.empty())
        {
            setAuthority(hostView.data());
        }
        else
        {
            replace(m_hostDelimited, hostView);
        }
    }

    std::string_view Uri::port() const
    {
        return m_port;
    }

    void Uri::setPort(const std::string& port)
    {
        if (m_port.empty())
        {
            if (m_host.empty())
            {
                throw std::runtime_error{ "cannot set port on URI without a host" };
            }

            std::string hostAndPort;
            hostAndPort += m_host;
            hostAndPort += ':';
            hostAndPort += port;
            replace(m_host, hostAndPort);
        }
        else
        {
            if (port.empty())
            {
                std::string_view portAndSeparator{ m_port.data() - 1, m_port.size() + 1 };
                replace(portAndSeparator, port);
            }
            else
            {
                replace(m_port, port);
            }
        }
    }

    std::string_view Uri::path() const
    {
        return m_path;
    }

    void Uri::setPath(const std::string& path)
    {
        if (!path.empty() && path.front() != '/')
        {
            throw std::runtime_error{ "URI path to set does not begin with '/': " + std::string{ path } };
        }

        if (m_path.empty())
        {
            m_uriStr += path;
            parse();
        }
        else
        {
            replace(m_path, path);
        }
    }

    void Uri::replace(std::string_view part, std::string_view replacement)
    {
        m_uriStr.replace(static_cast<size_t>(part.data() - m_uriStr.data()), part.size(), replacement);
        parse();
    }

    void Uri::parse()
    {
        try
        {
            m_userName = {};
            m_userPassword = {};
            m_hostDelimited = {};
            m_host = {};
            m_port = {};

            std::string_view remainder = m_uriStr;
            std::tie(m_scheme, remainder) = split_right_at_first_of(remainder, "://", false);

            if (m_scheme.empty())
            {
                std::tie(m_scheme, m_path) = split_right_at_first_of(remainder, ":", false);
            }
            else
            {
                std::tie(m_authority, m_path) = split_before_first_of(remainder, "/", false);

                if (!m_authority.empty())
                {
                    std::string_view userInfo;
                    std::tie(userInfo, remainder) = split_right_at_first_of(m_authority, "@", false);

                    if (!userInfo.empty())
                    {
                        std::tie(m_userName, m_userPassword) = split_left_at_first_of(userInfo, ":", false);
                    }

                    if (starts_with(remainder, "["))
                    {
                        std::tie(m_hostDelimited, m_port) = split_after_first_of(remainder, "]");

                        if (starts_with(m_port, ":"))
                        {
                            m_port.remove_prefix(1);
                        }

                        m_host = m_hostDelimited.substr(1, m_hostDelimited.size() - 2);
                    }
                    else
                    {
                        std::tie(m_host, m_port) = split_left_at_first_of(remainder, ":", false);
                        m_hostDelimited = m_host;
                    }
                }
            }

        }
        catch (const std::exception& e)
        {
            throw std::runtime_error{ "could not parse URI string '" + std::string{ m_uriStr } + "' -> " + e.what() };
        }
    }
}