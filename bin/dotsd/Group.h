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

        ConnectionList m_connections;
    public:

        Group(const string& name);
        ~Group() = default;

        void handleJoin(io::Connection* connection);
        void handleLeave(io::Connection* connection);
        void handleKill(io::Connection* connection);

        void deliver(const DotsTransportHeader& transportHeader, const Transmission& transmission);
    };

    typedef string GroupKey;
}
