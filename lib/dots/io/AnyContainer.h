#pragma once

#include "dots/cpp_config.h"
#include "Container.h"
#include <map>

namespace dots {

struct AnyElement
{
    struct LessThan
    {
        const type::StructDescriptor* td = nullptr;
        LessThan(const type::StructDescriptor* td): td(td)
        {}

        bool operator() (const AnyElement& l, const AnyElement& r) const
        {
            return td->lessThan(l.data, r.data);
        }
    };

    CloneInformation information;
    Typeless data;

    AnyElement() = default;
    AnyElement(CTypeless data, const CloneInformation& ci)
            : information(ci), data((Typeless)data)
    {}
};

class AnyContainerCbd
{
public:
    CTypeless receivedData;
    const AnyElement& element;
    const DotsHeader& header;
    const type::StructDescriptor* td;
    const Mt mt;
};

class AnyContainer
{

    typedef std::set<AnyElement, AnyElement::LessThan> container_type;

    container_type m_container;

public:
    typedef pnxs::Signal<void (const AnyContainerCbd&)> signal_type;

    AnyContainer(const type::StructDescriptor*);
    AnyContainer(const AnyContainer&);
    ~AnyContainer();

    AnyContainer& operator=(const AnyContainer&) = delete;

    size_t size() const;
    bool empty() const;
    void clear();
    void copy(const AnyContainer&);

    typedef container_type::const_iterator const_iterator;

    const_iterator begin() const;
    const_iterator end() const;
    const_iterator find(const AnyElement& e) const;

    Mt process(const DotsHeader &, Typeless, const signal_type * = nullptr);

    const type::StructDescriptor* td() const;

};

class AnyContainerPool
{
    typedef std::map<string, AnyContainer> pool_type;
    pool_type m_pool;

public:
    AnyContainer* getContainer(const string& name);
    const AnyContainer* getConstContainer(const string& name) const;
    pool_type& getPool();
    const pool_type& getPool() const;
};

}