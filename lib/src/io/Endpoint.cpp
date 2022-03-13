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
