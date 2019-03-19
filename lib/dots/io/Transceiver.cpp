#include <dots/io/AnyContainer.h>
#include "Transceiver.h"
#include "dots/type/Registry.h"
#include "DotsMsgConnect.dots.h"

namespace dots
{

Publisher* onPublishObject = nullptr;

Transceiver::Transceiver()
//: m_receiver(FUN(*this, onReceivedMessage))
{
    connection().onConnected.connect(FUN(*this, onConnect));
    connection().onEarlyConnect.connect(FUN(*this, onEarlySubscribe));
    connection().onReceiveMessage.connect(FUN(m_dispatcher, dispatchMessage));

    onPublishObject = this;
//    connection().onDisconnected
}

bool Transceiver::start(const string &name, const string &host, int port, DotsSocketPtr dotsSocket)
{
    LOG_DEBUG_S("start transceiver");

    // start communication
    if (connection().start(name, host, port, dotsSocket))
    {
       // m_receiver.start(connection().socket());
        // publish types
        return true;
    }

    return false;
}

void Transceiver::stop()
{
    // stop communication
    connection().stop();
}

ServerConnection &Transceiver::connection()
{
    return m_serverConnection;
}


void Transceiver::publish(const type::StructDescriptor *td, CTypeless data, property_set what, bool remove)
{
    connection().publish(td, data, what, remove);
}

void Transceiver::onConnect()
{
    m_connected = true;
}

bool Transceiver::connected() const
{
    return m_connected;
}

Dispatcher &Transceiver::dispatcher()
{
    return m_dispatcher;
}

void Transceiver::onEarlySubscribe()
{
    TD_Traversal traversal;

    for (const auto& td : getDescriptors())
    {
        if (td->internal()) continue;

        traversal.traverseDescriptorData(td, [this](auto td, auto body) {
            this->connection().publishNs("SYS", td, body, td->validProperties(body), false);
        });
    }

    // Send all subscribes
    for (auto& e: dots::ContainerBase::allChained())
    {
        connection().joinGroup(e->td()->name());
        dispatcher().registerReceiver(e->td(), e);
    }

    // Send preloadClientFinished
    DotsMsgConnect cm;
    cm.preloadClientFinished(true);

    connection().publishNs("SYS", &cm._Descriptor(), &cm);
}

type::StructDescriptorSet Transceiver::getPublishedDescriptors() const
{
    type::StructDescriptorSet sds;

    for (const auto& e : dots::PublishedType::allChained())
    {
        auto td = type::Descriptor::registry().findStructDescriptor(e->td->name());
        if (not td) {
            throw std::runtime_error("struct decriptor not found for " + e->td->name());
            //td = type::toStructDescriptor(type::Descriptor::registry().registerType(e->t));
        }
        if (td) {
            sds.insert(td);
        } else
        {
            LOG_ERROR_S("td is NULL: " << e->td->name())
        }
    }
    return sds;
}

type::StructDescriptorSet Transceiver::getSubscribedDescriptors() const
{
    type::StructDescriptorSet sds;

    for (const auto& e : dots::SubscribedType::allChained())
    {
        auto td = type::Descriptor::registry().findStructDescriptor(e->td->name());
        if (not td) {
            throw std::runtime_error("struct decriptor1 not found for " + e->td->name());
            //td = type::toStructDescriptor(type::Descriptor::registry().registerType(e->t));
        }
        if (td) {
            sds.insert(td);
        } else
        {
            LOG_ERROR_S("td is NULL: " << e->td->name());
        }
    }
    return sds;
}

type::StructDescriptorSet Transceiver::getDescriptors() const
{
    type::StructDescriptorSet sds;

    sds.merge(getPublishedDescriptors());
    sds.merge(getSubscribedDescriptors());

    return sds;
}

void Transceiver::subscribeDescriptors()
{
    //TODO: implement or cleanup?
    /*
    dispatcher().addReceiver<EnumDescriptorData>([registry](const EnumDescriptorData::Cbd& cbd) {
        registry;
    });
    dispatcher().addReceiver<StructDescriptorData>();
*/
}

Transceiver& transceiver()
{
    static Transceiver tranceiver;
    return tranceiver;
}

void publish(const type::StructDescriptor* td, CTypeless data, property_set what, bool remove)
{
    onPublishObject->publish(td, data, what, remove);
}

ServerConnection& gcomm()
{
    return transceiver().connection();
}

}