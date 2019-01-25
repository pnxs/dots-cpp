#include "AnyContainer.h"

#include "dots/type/Registry.h"
#include "dots/type/StructDescriptor.h"
#include "StructDescriptorData.dots.h"

namespace dots
{

AnyContainer::AnyContainer(const type::StructDescriptor *td)
:m_container(td)
{

}

AnyContainer::AnyContainer(const AnyContainer &r)
:m_container(r.td())
{
    copy(r);
}

AnyContainer::~AnyContainer()
{
    for(auto& element : m_container)
    {
        td()->Delete(element.data);
    }
    m_container.clear();
}

size_t AnyContainer::size() const
{
    return m_container.size();
}

bool AnyContainer::empty() const
{
    return m_container.empty();
}

void AnyContainer::clear()
{
    for(const auto& e : m_container)
    {
        td()->Delete(e.data);
    }
    m_container.clear();
}

void AnyContainer::copy(const AnyContainer & r)
{
    m_container = r.m_container;

    for (const auto& e : m_container)
    {
        Typeless data = td()->New();
        td()->copy(data, e.data);
        Writeable(e.data) = data;
    }

}

AnyContainer::const_iterator AnyContainer::begin() const
{
    return m_container.begin();
}

AnyContainer::const_iterator AnyContainer::end() const
{
    return m_container.end();
}

AnyContainer::const_iterator AnyContainer::find(const AnyElement &e) const
{
    return m_container.find(e);
}

Mt AnyContainer::process(const DotsHeader &header, Typeless data, const signal_type *signal)
{
    auto now = pnxs::SystemNow();
    Mt mt = Mt::remove;

    auto transmitted_keyfields = td()->validProperties(data) & td()->keys();

    if (transmitted_keyfields != td()->keys())
    {
        throw std::runtime_error("AnyContainer::process: not all key-fields of " + td()->name() + " are set. Set=" + transmitted_keyfields.to_string() + " Expected=" + td()->keys().to_string());
    }

    AnyElement element(data, {header.sentTime()});
    element.information.setModified(header.sentTime());

    if (not td()->cached())
    {
        if (not header.removeObj())
        {
            if (signal)
            {
                mt = Mt::create;
                // update element info
                Writable(element.information).setLastOperation(mt);

                Writable(element.information).setCreated(header.sentTime());
                Writable(element.information).setCreatedFrom(header.sender());

                Writable(element.information).setLastUpdateFrom(header.sender());
                Writable(element.information).setModified(header.sentTime());
                Writable(element.information).setLocalUpdateTime(now);

                (*signal)({element.data, element, header, td(), mt});
            }
            return Mt::create;
        }
        else
        {
            return Mt::remove;
        }
    }

    if (header.removeObj())
    {
        auto iter = m_container.find(element);
        if (iter != m_container.end())
        {
            if (signal)
            {
                // Update container-information because it's signalled to the user-code.
                Writeable(iter->information).setLastOperation(Mt::remove);
                Writeable(iter->information).setLastUpdateFrom(header.sender());
                Writeable(iter->information).setModified(header.sentTime());
                Writeable(iter->information).setLocalUpdateTime(now);

                (*signal)({iter->data, *iter, header, td(), mt});
            }

            td()->Delete(iter->data);
            m_container.erase(iter);
        }
        else
        {
            //TODO: Special case: remove without any existing object. Should this really be signalled to the user-code?
            // 2017-07-17: I think: No.
            // signal({data, element, header, td(), mt});
        }
    }
    else
    {
        auto rv = m_container.insert(element);

        const AnyElement& e = *rv.first;
        bool inserted = rv.second;

        if(inserted)
        {
            Writable(e.data) = td()->New(); // create new element data
            td()->swap(e.data, data, header.attributes()); // copy arrived data to element data
        }
        else
        {
            // incremental update element with received data attributes, except keys
            td()->swap(e.data, data, header.attributes() & ~td()->keys());
        }

        mt = inserted ? Mt::create : Mt::update;

        // update element info
        Writable(e.information).setLastOperation(mt);
        if (mt == Mt::create) {
            Writable(e.information).setCreated(header.sentTime());
            Writable(e.information).setCreatedFrom(header.sender());
        }
        Writable(e.information).setLastUpdateFrom(header.sender());
        Writable(e.information).setModified(header.sentTime());
        Writable(e.information).setLocalUpdateTime(now);

        if (signal)
        {
            (*signal)({e.data, e, header, td(), mt}); // post insert/update, invoke signal (callbacks)
        }
    }
    return mt;
}

const type::StructDescriptor *AnyContainer::td() const
{
    return m_container.key_comp().td;
}

AnyContainer *AnyContainerPool::getContainer(const string &name)
{
    auto it = m_pool.find(name);

    if (it == m_pool.end())
    {
        auto td = type::Descriptor::registry().findStructDescriptor(name);

        if (td == nullptr)
        {
            LOG_WARN_S("descriptor not found in pool (" << name << ")");
            return nullptr;
        }

        it = m_pool.insert({td->name(), td}).first;
    }

    return &it->second;
}

const AnyContainer* AnyContainerPool::getConstContainer(const string& name) const
{
    auto it = m_pool.find(name);
    if (it == m_pool.end())
    {
        return nullptr;
    }

    return &it->second;
}

AnyContainerPool::pool_type &AnyContainerPool::getPool()
{
    return m_pool;
}

const AnyContainerPool::pool_type &AnyContainerPool::getPool() const
{
    return m_pool;
}

}