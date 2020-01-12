#pragma once

#include <set>
#include "dots/cpp_config.h"
#include <dots/io/services/Transmission.h>
#include <dots/dots_base.h>
#include "DotsMember.dots.h"
#include <DotsTransportHeader.dots.h>

namespace dots::io
{
    struct Connection;
}

namespace dots {

typedef ClientId GroupMember;

/**
 * A representation of a messaging-group in DOTS.
 * A peer can subscribe to a group and gets all updates to this
 * group.
 */
class Group
{
    typedef std::vector<io::Connection*> ConnectionList;
    const string m_name;
    std::set<GroupMember> m_membersList; // all groups members

    /**
     * Search for member in membersList.
     * @param key member-key
     * @return member-point or NULL
     */
    const GroupMember *getMember(const GroupMember &key)
    {
        auto it = m_membersList.find(key);
        return it != m_membersList.end() ? &*it : NULL;
    }

    void removeConnection(io::Connection *connection);

    ConnectionList m_connections; // local group members
public:

    Group(const string &name);
    ~Group();

    const string &name() const { return m_name; }

    void handleJoin(io::Connection *connection);
    void handleLeave(io::Connection *connection);
    void handleKill(io::Connection *connection);

    void deliver(const DotsTransportHeader& transportHeader, const Transmission& transmission);
};

typedef string GroupKey;

}
