#include "Dispatcher.h"
#include "dots/type/Registry.h"
#include "dots/io/serialization/CborNativeSerialization.h"

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

void Dispatcher::dispatchMessage(const ReceiveMessageData &rmd)
{
    auto& typeName = rmd.group;

    auto td = type::Descriptor::registry().findStructDescriptor(typeName);
    if (td)
    {
        // create object to deserialize data into
        auto obj = td->make_shared();

        from_cbor(rmd.data, rmd.length, td, obj.get());

        const TypedescSignalPtr& signal = m_typeSignalMap[typeName];
        const auto& typelessSignal = m_typelessSignalMap[typeName];
        if (signal || typelessSignal)
        {
            if (not td->internal())
            {
                (*m_statistics.packages)++;
                (*m_statistics.bytes) += rmd.length;
            }

            DotsHeader header(rmd.header);

            if (not header.removeObj.isValid()) {
                header.removeObj = false;
            }

            header.sender = rmd.sender;
            header.sentTime = rmd.sentTime;
            header.isFromMyself = rmd.isFromMyself;

            if (typelessSignal)
            {
                TypelessCbd typelessCbd{header, typelessSignal->td(), obj.get(), obj, rmd.length};
                (*typelessSignal)(&typelessCbd);
            }

            if (signal)
            {
                auto container = signal->container();

                if (container) {
                    container->processTypeless(header, (Typeless) obj.get(), *signal);
                }
            }

        }
        else
        {
            LOG_DEBUG_S("no receiver registered for type " << typeName);
        }

    }
    else
    {
        LOG_WARN_S("unable to dispatch message (" << rmd.group << "), because TD is not in pool.");
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