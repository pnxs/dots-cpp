#pragma once

#include "dots/cpp_config.h"
#include "Group.h"
#include <unordered_map>

namespace dots {

/*!
 * Managed all a set of groups.
 * Allows Connections to join or leave groups.
 */
class GroupManager
{
    //std::unordered_map<GroupKey, Group *, GroupHash> m_allGroups;
    std::unordered_map<GroupKey, Group *> m_allGroups;
public:

    GroupManager();
    virtual ~GroupManager() = default;

    /*!
     * Find Group from group-key
     * @param groupKey key of group to search for.
     * @return Group-Pointer. Null of group-key was not found.
     */
    virtual Group *getGroup(const GroupKey &groupKey)
    {
        auto it = m_allGroups.find(groupKey);
        return it != m_allGroups.end() ? it->second : NULL;
    }

    /*!
     * Add Connection to group. Create group if it does not exist jet.
     * @param groupKey
     * @param connection
     */
    virtual void handleJoin(const GroupKey& groupKey, Connection *connection);

    /*!
     * Remove a Connection from a group.
     * @param groupKey
     * @param connection
     */
    virtual void handleLeave(const GroupKey& groupKey, Connection *connection);

    /*!
     * Removes a killed Connection from all groups.
     * @param connection
     */
    virtual void handleKill(Connection *connection);
};

}