#pragma once
#include <string>
#include <dots/asio_forward.h>
#include <dots/tools/Uri.h>

namespace dots::io
{
    struct Endpoint : tools::Uri
    {
        using Uri::Uri;

        template <typename InternetProtocol>
        Endpoint(const std::string& scheme, const asio::ip::basic_endpoint<InternetProtocol>& endpoint) :
            Endpoint(scheme, endpoint.address().to_string(), endpoint.port())
        {
            /* do nothing */
        }

        template <typename InternetProtocol>
        Endpoint(const asio::ip::basic_endpoint<InternetProtocol>& endpoint) :
            Endpoint([]() -> std::string
            {
                constexpr bool IsTcp = std::is_same_v<InternetProtocol, asio::ip::tcp>;
                static_assert(std::is_same_v<InternetProtocol, asio::ip::tcp>, "unsupported IP endpoint protocol");

                if constexpr (IsTcp)
                {
                    return "tcp";
                }
                else
                {
                    return {};
                }
            }(), endpoint)
        {
            /* do nothing */
        }

        static std::vector<Endpoint> FromStrings(const std::string& uriStrs);
    };
}
