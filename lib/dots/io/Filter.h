#pragma once

#include <regex>
#include <vector>

namespace dots
{

class Filter
{
public:

    bool operator()(const std::string &name);

    void setWhitelist(const std::vector<std::string> &whitelist);
    void setBlacklist(const std::vector<std::string> &blacklist);


private:
    std::vector<std::regex> m_whitelistTypes;
    std::vector<std::regex> m_blacklistTypes;
};

}