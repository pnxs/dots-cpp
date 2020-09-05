#pragma once
#include <string>

namespace dots::io
{
    struct Medium
    {
        Medium(std::string category, std::string endpoint);
        Medium(const Medium& other) = default;
        Medium(Medium&& other) = default;
        ~Medium() = default;

        Medium& operator = (const Medium& rhs) = default;
        Medium& operator = (Medium&& rhs) = default;

        const std::string& category() const;
        const std::string& endpoint() const;

    private:

        std::string m_category;
        std::string m_endpoint;
    };
}