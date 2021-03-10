#pragma once
#include <string>
#include <memory>
#include <filesystem>
#include <boost/asio.hpp>
#include <boost/filesystem/path.hpp>
#include <dots/tools/Uri.h>

namespace dots::io
{
    struct Endpoint : tools::Uri
    {
        using Uri::Uri;
        Endpoint(const std::string& scheme, const std::filesystem::path& path);
        Endpoint(const std::string& scheme, const boost::filesystem::path& path);
        Endpoint(const Endpoint& other) = default;
        Endpoint(Endpoint&& other) = default;
        ~Endpoint() = default;

        Endpoint& operator = (const Endpoint& rhs) = default;
        Endpoint& operator = (Endpoint&& rhs) = default;

        void setPath(const std::filesystem::path& path);
        void setPath(const boost::filesystem::path& path);
    };
}