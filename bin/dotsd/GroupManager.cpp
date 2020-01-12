#include "GroupManager.h"

namespace dots {

void GroupManager::handleJoin(const string &groupKey, io::Connection *connection)
{
    auto group = getGroup(groupKey);
    if (group == nullptr)
    {
        group = new Group(groupKey);
        m_allGroups.insert({group->name(), group});
    }

    group->handleJoin(connection);
}

void GroupManager::handleLeave(const string &groupKey, io::Connection *connection)
{
    auto group = getGroup(groupKey);
    if (group == nullptr)
    {
        LOG_ERROR_S("group does not exist");
        return;
    }

    group->handleLeave(connection);
}

void GroupManager::handleKill(io::Connection *connection)
{
    for (auto& i : m_allGroups)
    {
        auto& group = i.second;
        group->handleKill(connection);
    }
}

}