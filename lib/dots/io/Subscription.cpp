#include "Subscription.h"

namespace dots
{


PublishedType::PublishedType(const type::StructDescriptor* td)
:td(td)
{
    LOG_DEBUG_S("PubType: " << td->name());
}

SubscribedType::SubscribedType(const type::StructDescriptor* td)
:td(td)
{
    LOG_DEBUG_S("SubType: " << td->name());
}

Subscription::Subscription(const type::StructDescriptor *td, const pnxs::SignalConnection &sc)
:m_td(td), m_sc(sc)
{

}


void Subscription::unsubscribe() const
{
    m_sc.disconnect();
}

const type::StructDescriptor *Subscription::td() const
{
    return m_td;
}

}