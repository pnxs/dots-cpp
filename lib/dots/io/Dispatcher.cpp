#include "Dispatcher.h"
#include "dots/type/Registry.h"

namespace dots
{

Dispatcher::Dispatcher()
{
    m_statistics.packages(0);
    m_statistics.bytes(0);
}

Subscription
Dispatcher::addReceiver(const type::StructDescriptor *td, ContainerBase *cb, const Dispatcher::callback_type &callback)
{
    if (td == nullptr) {
        throw std::invalid_argument("addReceiver: td is nullptr");
    }
    TypedescSignalPtr signal = registerReceiver(td, cb);

    return Subscription(td, signal->connect(callback));
}

Dispatcher::TypedescSignalPtr Dispatcher::registerReceiver(const type::StructDescriptor *td, ContainerBase *cb)
{
    TypedescSignalPtr& signal = m_typeSignalMap[td->name()];

    LOG_DEBUG_S("registerReceiver: " << td->name());

    if (not signal)
    {
        signal = std::make_shared<TypedescSignal>(td, cb);
    } else
    {
        // This is not a warning, it's allowed for the using program to register as many receivers for one type
        // as needed
        //LOG_WARN_S("already registered receiver for " << td->name());
    }

    return signal;
}

void Dispatcher::dispatchMessage(const DotsHeader& header, const type::AnyStruct& instance)
{
    const TypedescSignalPtr& signal = m_typeSignalMap[header.typeName];
    const auto& typelessSignal = m_typelessSignalMap[header.typeName];

    if (signal || typelessSignal)
    {
        if (not instance->_descriptor().internal())
        {
            (*m_statistics.packages)++;
            (*m_statistics.bytes) += 1; // TODO: find other form of metric (e.g. effective memory size)
        }

        if (typelessSignal)
        {
            TypelessCbd typelessCbd{ header, instance };
            (*typelessSignal)(&typelessCbd);
        }

        if (signal)
        {
            auto container = signal->container();

            if (container) {
                container->processTypeless(header, instance, *signal);
            }
        }
    }
    else
    {
        LOG_DEBUG_S("no receiver registered for type " << header.typeName);
    }
}

Subscription
Dispatcher::addTypelessReceiver(const type::StructDescriptor *td, const Dispatcher::typeless_callback_type &callback)
{
    TypedescSignalPtr signal = registerTypelessReceiver(td);
    return Subscription(td, signal->connect(*(const callback_type*)&callback));
}

const DotsStatistics &Dispatcher::statistics() const
{
    return m_statistics;
}

Dispatcher::TypedescSignalPtr Dispatcher::registerTypelessReceiver(const type::StructDescriptor *td)
{
    TypedescSignalPtr& signal = m_typelessSignalMap[td->name()];

    LOG_DEBUG_S("registerTypelessReceiver: " << td->name());

    if (not signal)
    {
        signal = std::make_shared<TypedescSignal>(td, nullptr);
    } else{
        LOG_WARN_S("already registered typeless-receiver for " << td->name());
    }
    return signal;
}

}