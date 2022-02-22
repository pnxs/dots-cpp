#include <dots/io/Endpoint.h>

namespace dots::io
{
    Endpoint::Endpoint(const std::string& scheme, const std::filesystem::path& path) :
        Endpoint(scheme, path.string())
    {
        /* do nothing */
    }

    Endpoint::Endpoint(const std::string& scheme, const boost::filesystem::path& path) :
        Endpoint(scheme, path.string())
    {
        /* do nothing */
    }

    void Endpoint::setPath(const std::filesystem::path& path)
    {
        Uri::setPath(path.string());
    }

    void Endpoint::setPath(const boost::filesystem::path& path)
    {
        Uri::setPath(path.string());
    }

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
