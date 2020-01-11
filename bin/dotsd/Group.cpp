#include "Group.h"
#include "DotsMember.dots.h"
#include "Connection.h"
#include "ConnectionManager.h"

namespace dots {

Group::Group(const string &name)
:m_name(name)
{
}

Group::~Group()
{
}

/**
 * Add peer to membersList and push Connection into connection-list.
 * Send join-message to all registered connection, that want MemberMessages
 * @param groupMember
 * @param connection
 */
void Group::handleJoin(Connection *connection)
{
    auto& groupMember = connection->id();
    auto member = getMember(groupMember);

    if (member)
    {
        LOG_WARN_S(connection->name() << " is already member of group " << name());
        return;
    }

    m_membersList.insert(groupMember);

    if (connection)
    {
        m_connections.push_back(connection);
    }
}

void Group::handleLeave(Connection *connection)
{
    auto& groupMember = connection->id();
    auto member = getMember(groupMember);

    if (not member)
    {
        LOG_WARN_P("member does not exist");
        return;
    }

    if (not connection)
    {
        LOG_WARN_S("member has no local connections in group");
    } else {
        removeConnection(connection);
    }

    m_membersList.erase(*member);
}

void Group::handleKill(Connection *connection)
{
    auto& groupMember = connection->id();
    auto member = getMember(groupMember);
    if (member == nullptr) return;

    removeConnection(connection);

    m_membersList.erase(*member);
}

void Group::removeConnection(Connection *connection)
{
    if (not connection)
    {
        LOG_WARN_P("invalid connection");
        return;
    }

    for (auto& i : m_connections)
    {
        if (i == connection)
        {
            i = m_connections.back();
            m_connections.pop_back();
            return;
        }
    }
}

void Group::deliver(const DotsTransportHeader& transportHeader, const Transmission& transmission)
{
    LOG_DEBUG_S("deliver message group:" << this << "(" << name() << ")");
    // Dispatch message to all connections, registered to the group
    for(const auto connection : m_connections)
    {
        if(connection != NULL)
        {
            string ns;

            if (transportHeader.nameSpace.isValid()) ns = transportHeader.nameSpace;
            LOG_DATA_S("send to connection " << connection->id() << " ns=" << ns << " grp=" << transportHeader.destinationGroup);
            if (connection->state() == DotsConnectionState::connected
                or connection->state() == DotsConnectionState::suspended)
            {
                connection->transmit(transportHeader, transmission);
            }
        }
    }
}

}