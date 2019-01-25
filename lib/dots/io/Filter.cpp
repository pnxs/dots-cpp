#include "Filter.h"


namespace dots
{

void Filter::setBlacklist(const std::vector<std::string> &blacklist)
{
    for (auto& item : blacklist)
    {
        m_blacklistTypes.emplace_back(item);
    }
}

void Filter::setWhitelist(const std::vector<std::string> &whitelist)
{
    for (auto& item : whitelist)
    {
        m_whitelistTypes.emplace_back(item);
    }
}

bool Filter::operator()(const std::string &name)
{
    if (not m_whitelistTypes.empty())
    {
        // When whitelist is not empty, only accept types, that match with the whitelist
        bool matched = false;

        for (auto& re : m_whitelistTypes)
        {
            if (std::regex_match(name, re))
            {
                matched = true;
                break;
            }
        }

        if (not matched)
        {
            return false;
        }
    }

    if (not m_blacklistTypes.empty())
    {
        // When blacklist is not empty, reject types, that match the blacklist
        for (auto& re : m_blacklistTypes)
        {
            if (std::regex_match(name, re))
            {
                return false;
            }
        }
    }

    return true;
}

}