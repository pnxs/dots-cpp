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

namespace dots
{
    typedef ClientId GroupMember;

    class Group
    {
        typedef std::vector<io::Connection*> ConnectionList;
        const string m_name;
        std::set<GroupMember> m_membersList;

        const GroupMember* getMember(const GroupMember& key)
        {
            auto it = m_membersList.find(key);
            return it != m_membersList.end() ? &*it : NULL;
        }

        void removeConnection(io::Connection* connection);

        ConnectionList m_connections;
    public:

        Group(const string& name);
        ~Group();

        const string& name() const { return m_name; }

        void handleJoin(io::Connection* connection);
        void handleLeave(io::Connection* connection);
        void handleKill(io::Connection* connection);

        void deliver(const DotsTransportHeader& transportHeader, const Transmission& transmission);
    };

    typedef string GroupKey;
}
