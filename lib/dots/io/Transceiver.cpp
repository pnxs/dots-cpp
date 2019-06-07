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
    connection().onReceiveMessage.connect(FUN(m_dispatcher, dispatch));

    onPublishObject = this;
//    connection().onDisconnected
}

bool Transceiver::start(const string &name, channel_ptr_t channel)
{
    LOG_DEBUG_S("start transceiver");

    // start communication
    if (connection().start(name, channel))
    {
       // m_receiver.start(connection().channel());
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

const ContainerPool& Transceiver::pool() const
{
	return m_dispatcher.pool();
}

const Container<>& Transceiver::container(const type::StructDescriptor& descriptor)
{
	return m_dispatcher.container(descriptor);
}

Subscription Transceiver::subscribe(const type::StructDescriptor& descriptor, receive_handler_t<>&& handler)
{
	connection().joinGroup(descriptor.name());
	return m_dispatcher.subscribe(descriptor, std::move(handler));
}

Subscription Transceiver::subscribe(const type::StructDescriptor& descriptor, event_handler_t<>&& handler)
{
	connection().joinGroup(descriptor.name());
	return m_dispatcher.subscribe(descriptor, std::move(handler));
}

Subscription Transceiver::subscribe(const std::string_view& name, receive_handler_t<>&& handler)
{
	return subscribe(getDescriptorFromName(name), std::move(handler));
}

Subscription Transceiver::subscribe(const std::string_view& name, event_handler_t<>&& handler)
{
	return subscribe(getDescriptorFromName(name), std::move(handler));
}

ServerConnection &Transceiver::connection()
{
    return m_serverConnection;
}


void Transceiver::publish(const type::StructDescriptor *td, const type::Struct& instance, property_set what, bool remove)
{
    connection().publish(td, instance, what, remove);
}

void Transceiver::onConnect()
{
    m_connected = true;
}

bool Transceiver::connected() const
{
    return m_connected;
}

void Transceiver::onEarlySubscribe()
{
    TD_Traversal traversal;

    for (const auto& td : getDescriptors())
    {
        if (td->internal()) continue;

        traversal.traverseDescriptorData(td, [this](auto td, auto body) {
            this->connection().publishNs("SYS", td, *reinterpret_cast<const type::Struct*>(body), td->validProperties(body), false);
        });
    }

    // Send all subscribes
    for (auto& descriptor: getSubscribedDescriptors())
    {
        connection().joinGroup(descriptor->name());
    }

    // Send preloadClientFinished
    DotsMsgConnect cm;
    cm.preloadClientFinished(true);

    connection().publishNs("SYS", &cm._Descriptor(), cm);
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

const type::StructDescriptor& Transceiver::getDescriptorFromName(const std::string_view& name) const
{
	const type::Descriptor* descriptor = type::Descriptor::registry().fromWireName(name.data());

	if (descriptor == nullptr)
	{
		throw std::logic_error{ "could not find a struct type with name: " + std::string{ name.data() } };
	}

	if (descriptor->dotsType() != type::DotsType::Struct)
	{
		throw std::logic_error{ "type with name is not a struct type: " + std::string{ name.data() } };
	}

	return *static_cast<const type::StructDescriptor*>(descriptor);
}

}