// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
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
                static_assert(IsTcp, "unsupported IP endpoint protocol");
                (void)IsTcp;

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
