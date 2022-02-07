#pragma once
#include <string>
#include <filesystem>
#include <dots/asio.h>
#include <boost/filesystem/path.hpp>
#include <dots/tools/Uri.h>

namespace dots::io
{
    struct Endpoint : tools::Uri
    {
        using Uri::Uri;
        Endpoint(const std::string& scheme, const std::filesystem::path& path);
        Endpoint(const std::string& scheme, const boost::filesystem::path& path);

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

        void setPath(const std::filesystem::path& path);
        void setPath(const boost::filesystem::path& path);
    };
}
