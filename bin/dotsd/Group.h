#pragma once

#include "dots/cpp_config.h"
#include "DotsMember.dots.h"
#include "dots/io/Message.h"

namespace dots {

typedef ClientId GroupMember;
class Connection;

/**
 * A representation of a messaging-group in DOTS.
 * A peer can subscribe to a group and gets all updates to this
 * group.
 */
class Group
{
    typedef std::vector<Connection*> ConnectionList;
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

    void sendLeave(Connection *connection);
    void sendMemberMessage(const DotsMemberEvent &event, const ClientId &changeMember);

    void removeConnection(Connection *connection);

    ConnectionList m_connections; // local group members
public:

    Group(const string &name);
    virtual ~Group();

    virtual bool empty() const { return m_membersList.empty(); }
    virtual const string &name() const { return m_name; }

    virtual void handleJoin(Connection *connection);
    virtual void handleLeave(Connection *connection);
    virtual void handleKill(Connection *connection);

    virtual const ConnectionList& connections() const { return m_connections; }

    virtual void deliverMessage(const Message&);
};

typedef string GroupKey;

}
