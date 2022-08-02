// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/io/Endpoint.h>

namespace dots::io
{
    std::vector<Endpoint> Endpoint::FromStrings(const std::string& uriStrs)
    {
        std::vector<Endpoint> endpoints;

        for (const Uri& uri : Uri::FromStrings(uriStrs))
        {
            endpoints.emplace_back(std::string{ uri.uriStr() });
        }

        return endpoints;
    }
}
