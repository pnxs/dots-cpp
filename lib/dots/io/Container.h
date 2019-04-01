#pragma once

#include "dots/cpp_config.h"

#include <dots/eventloop/Chrono.h>
#include "dots/type/StructDescriptor.h"
#include "Chained.h"
#include "Subscription.h"
#include <dots/functional/signal.h>
#include <set>

#include "DotsHeader.dots.h"
#include "DotsCloneInformation.dots.h"

namespace dots
{

/**
 * DOTS Modification Type
 * Describes, if a callback is because an object was created, updated or removed.
 */
typedef DotsMt Mt;

struct CloneInformation: public types::DotsCloneInformation
{

    CloneInformation() {
        lastOperation(DotsMt::create);
    }

    CloneInformation(const TimePoint& created)
    {
        lastOperation(DotsMt::create);
		DotsCloneInformation::created(created);
    }
};

template<class T>
class Clone: public T
{
public:
    CloneInformation _clone_metainfo;

    Clone() = default;
    Clone(const T& data, const CloneInformation& info)
            :T(data), _clone_metainfo(info)
    {}

    Clone& operator=(const T& key)
    {
        T::operator=(key);
        return *this;
    }

};

struct TypelessCbd
{
    const types::DotsHeader& header;
    const type::StructDescriptor* td;
    Typeless data;
    std::shared_ptr<void> dataPtr;
    size_t length = 0;
};

/**
 * CallBackData,
 * used by the container-user-callback
 */
template<class T>
struct Cbd
{
private:
    const CloneInformation& information;

    const T& data;
public:
    const DotsHeader& header;
    const Mt mt;

    Cbd(const Clone<T>& clone, const types::DotsHeader& header, Mt mt)
            : information(clone._clone_metainfo), data(clone), header(header), mt(mt)
    {}

    Cbd(const T& data, const CloneInformation& information, const types::DotsHeader& header, Mt mt)
            : information(information), data(data), header(header), mt(mt)
    {
    }

    bool isCreate() const { return mt == Mt::create; }
    bool isUpdate() const { return mt == Mt::update; }
    bool isRemove() const { return mt == Mt::remove; }

    bool isOwnUpdate() const { return header.isFromMyself; }

    /**
     * @return contained DOTS object
     */
    const T& operator()() const { return data; }

    /**
     * @return Set of properties, that are transmitted within this update
     */
    const property_set newProperties() const { return header.attributes; }

    /**
     * @return Set of properties, that are updated and valid within this update
     */
    const property_set updatedProperties() const { return newProperties() & data._validProperties(); }
};

template<class T>
class Container
{
protected:
    typedef std::set<Clone<T>> container_type;
    container_type m_container;

    mutable Clone<T> m_keyHelper;

public:
    size_t size() const { return m_container.size(); }
    bool empty() const { return m_container.empty(); }
    void clear() { m_container.clear(); }

    typedef typename container_type::iterator iterator;
    typedef typename container_type::const_iterator const_iterator;

    iterator begin() const { return m_container.begin(); }
    iterator end() const { return m_container.end(); }
    const_iterator cbegin() const { return m_container.cbegin(); }
    const_iterator cend() const { return m_container.cend(); }

    const_iterator findIter(const T& key) const
    {
        m_keyHelper = key;
        return m_container.find(m_keyHelper);
    }

    const Clone<T>* find(const T& key) const
    {
        auto iter = findIter(key);
        return (iter != end()) ? &*iter : nullptr;
    }
};

class ContainerBase: public Chained<ContainerBase>
{
    const type::StructDescriptor* m_td;

public:
    typedef pnxs::Signal<void (CTypeless cbd)> signal_type;

    ContainerBase(const type::StructDescriptor* td);

    virtual size_t typelessSize() const = 0;
    virtual bool processTypeless(const types::DotsHeader& header, Typeless data, const signal_type& signal) = 0;

    const type::StructDescriptor* td() const;
};

template<class T>
static inline
T& Writeable(const T& d) { return const_cast<T&>(d); }

template <class T>
class ContainerEx: public ContainerBase, public Container<T>
{
private:
    bool insert(const DotsHeader& header, T& data, const signal_type& signal)
    {
		auto rc = this->m_container.insert({ data, *header.sentTime });

        const auto& item = *rc.first;
        bool inserted = rc.second;

        if (not inserted)
        {
            // Update element with received attributes
            Writeable(item)._swap(data, data._validProperties() - T::_KeyProperties());
        }

        Cbd<T> cbd(item, header, inserted ? Mt::create : Mt::update );

        // Call user-code
        signal(&cbd);

        return inserted;
    }

    bool remove(const types::DotsHeader& header, const T& data, const signal_type& signal)
    {
        auto iter = this->findIter(data);

        if (iter != this->m_container.end())
        {
            Cbd<T> cbd(*iter, header, Mt::remove);

            // Call user-code
            signal(&cbd);

            this->m_container.erase(iter);

            return true;
        }

        return false;
    }

public:
    ContainerEx(): ContainerBase(&T::_Descriptor())
    {
    }

    /**
     * Moves data into container and calls signal
     * @param header
     * @param data
     * @param signal
     * @return true if container changed size (items)
     */
    bool process(const types::DotsHeader& header, T& data, const signal_type& signal)
    {
        //TODO: convert to if constexpr when C++17 can be used.
        if (T::_IsCached())
        {
            if (header.removeObj)
            {
                return remove(header, data, signal);
            }
            else
            {
                return insert(header, data, signal);
            }
        }
        else
        {
            Clone<T> item{data, *header.sentTime};
            Cbd<T> cbd(item, header, Mt::create);
            // Call user-code
            signal(&cbd);

			return false;
        }
    }

    size_t typelessSize() const final { return this->size(); }

    bool processTypeless(const types::DotsHeader& header, Typeless data, const signal_type& signal) final
    {
        return process(header, *reinterpret_cast<T*>(data), signal);
    }
};

/**
 * This template creates for every DOTS-type that is used with dots::C or dots::rC
 * a container.
 */
template<class T>
struct StaticContainer
{
    static ContainerEx<T> container;
};

template<class T>
ContainerEx<T> StaticContainer<T>::container;

/**
 * API to get a writeable access to a type-specific container
 * Usage:
 * dots::rC<TypeName>().methodOfContainer();
 *
 * @tparam T DOTS-Type
 * @return reference to container
 */
template<class T>
ContainerEx<T>& rC() {
    registerTypeUsage<T, SubscribedType>();
    return StaticContainer<T>::container;
}

/**
 * API to get a read-only acces to a type-specific container
 * Usage:
 * dots::C<TypeName>().methodOfContainer(); (e.g. find())
 * @tparam T
 * @return const reference to container
 */
template<class T>
const ContainerEx<T>& C() {
    registerTypeUsage<T, SubscribedType>();
    return StaticContainer<T>::container;
}

}