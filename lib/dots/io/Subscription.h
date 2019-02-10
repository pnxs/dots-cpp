#pragma once

#include "Chained.h"
#include <dots/functional/signal.h>
#include <dots/type/StructDescriptor.h>
namespace dots
{

struct PublishedType: public Chained<PublishedType>
{
    const type::StructDescriptor* td;
    PublishedType(const type::StructDescriptor* td);
};

struct SubscribedType: public Chained<SubscribedType>
{
    const type::StructDescriptor* td;
    SubscribedType(const type::StructDescriptor* td);
};

/**
 * Helper template to create a instance of Listname-Class (S)
 * for every type T
 * @tparam T
 * @tparam S
 */
template<class T, class S>
class RegisterTypeUsage
{
public:
    static S& get()
    {
        return m_obj;
    }
private:
    static S m_obj;
};

template<class T, class S>
S RegisterTypeUsage<T, S>::m_obj(&T::_Descriptor());

/**
 * Registeres usage of type T in the given Chained-List
 *
 * @tparam T DOTS-Type to register
 * @tparam S Chained-List name
 */
template<class T, class S>
void registerTypeUsage() {
    RegisterTypeUsage<T, S>::get();
};

class Subscription
{
public:
    Subscription() = default;

    void unsubscribe() const;
    const type::StructDescriptor* td() const;

private:
    friend class Dispatcher;

    Subscription(const type::StructDescriptor* td, const pnxs::SignalConnection& sc);

    const type::StructDescriptor* m_td = nullptr;
    pnxs::SignalConnection m_sc;

};

}