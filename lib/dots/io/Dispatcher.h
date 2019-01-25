#pragma once

#include "dots/cpp_config.h"
#include "Container.h"
#include "Subscription.h"
#include <dots/functional/signal.h>
#include <unordered_map>
#include "ServerConnection.h"
#include "Subscription.h"

#include "DotsStatistics.dots.h"

namespace dots
{

class Dispatcher
{
private:
    class TypedescSignal: public pnxs::Signal<void (CTypeless)>
    {
    public:
        TypedescSignal(const type::StructDescriptor* td, ContainerBase* cb)
            : m_td(td), m_obj(td->New()), m_cb(cb)
        {}

        ~TypedescSignal()
        {
            m_td->Delete(m_obj);
        }

        const type::StructDescriptor* td() const
        {
            return m_td;
        }

        Typeless obj() const
        {
            return m_obj;
        }

        ContainerBase* container() const
        {
            return m_cb;
        }

    private:
        const type::StructDescriptor* m_td;
        Typeless m_obj;
        ContainerBase* m_cb;
    };

    typedef shared_ptr<TypedescSignal> TypedescSignalPtr;

    template <class T>
    static void wrapper(CTypeless cbd, const function<void (const Cbd<T>&)>& f)
    {
        //f(*reinterpret_cast<Cbd<T>*>(cbd));
        f(*(Cbd<T>*)(cbd));

    }
public:
    typedef function<void (CTypeless cbd)> callback_type;
    typedef function<void (const TypelessCbd* cbd)> typeless_callback_type;

    Dispatcher();

    /**
     * Dispatch the message to all registered receivers
     * @param cbd struct that contains the message and metadata
     */
    void dispatchMessage(const ReceiveMessageData& cbd);

    Subscription addTypelessReceiver(const type::StructDescriptor* td, const typeless_callback_type& callback);
    Subscription addReceiver(const type::StructDescriptor* td, ContainerBase* cb, const callback_type& callback);

    TypedescSignalPtr registerReceiver(const type::StructDescriptor* td, ContainerBase* cb);
    TypedescSignalPtr registerTypelessReceiver(const type::StructDescriptor* td);

    template<class T>
    Subscription addReceiver(const function<void (const Cbd<T>& cbd)>& callback)
    {
        C<T>(); // Create static container
        registerTypeUsage<T, SubscribedType>();

        static_assert(not T::isSubstructOnly(), "It is not allowed to subscribe a struct, that is marked with 'substruct_only'!");

        auto ret = addReceiver(T::_td(), &rC<T>(), bind(&wrapper<T>, _1, callback));

        if (C<T>().empty())
            return ret;

        DotsHeader dh;
        dh.setTypeName(T::_td()->name());
        dh.setRemoveObj(false);

        for (const auto& e : C<T>())
        {
            callback({e, dh, Mt::create});
        }

        return ret;
    }

    const DotsStatistics& statistics() const;
private:
    std::unordered_map<string, TypedescSignalPtr> m_typeSignalMap;
    std::unordered_map<string, TypedescSignalPtr> m_typelessSignalMap;
    DotsStatistics m_statistics;

};

}