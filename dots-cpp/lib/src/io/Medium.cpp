#include <dots/io/Medium.h>

namespace dots::io
{
    Medium::Medium(std::string category, std::string endpoint) :
        m_category{ std::move(category) },
        m_endpoint{ std::move(endpoint) }
    {
        /* do nothing */
    }

    const std::string& Medium::category() const
    {
        return m_category;
    }

    const std::string& Medium::endpoint() const
    {
        return m_endpoint;
    }
}