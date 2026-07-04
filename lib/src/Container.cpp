// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/Container.h>
#include <algorithm>
#include <numeric>
#include <dots/tools/hash.h>

namespace dots
{
    Container<type::Struct>::key_hash::key_hash(const type::StructDescriptor& descriptor)
    {
        for (const type::PropertyDescriptor& propertyDescriptor : descriptor.propertyDescriptors())
        {
            if (propertyDescriptor.isKey())
            {
                m_keyPropertyDescriptors.emplace_back(propertyDescriptor);
            }
        }
    }

    size_t Container<type::Struct>::key_hash::operator()(const type::Struct& s) const
    {
        const type::PropertyArea& area = s._propertyArea();
        size_t h = 0;

        for (const auto& propertyDescriptor_ : m_keyPropertyDescriptors)
        {
            const type::PropertyDescriptor& pd = propertyDescriptor_.get();
            const auto& v = area.getProperty<type::Typeless>(pd.offset());
            h = tools::hashCombine(h, pd.valueDescriptor().hash(v));
        }

        return h;
    }

    size_t Container<type::Struct>::key_hash::operator()(const type::AnyStruct& s) const
    {
        return (*this)(static_cast<const type::Struct&>(s));
    }

    Container<type::Struct>::key_equal::key_equal(const type::StructDescriptor& descriptor)
    {
        for (const type::PropertyDescriptor& propertyDescriptor : descriptor.propertyDescriptors())
        {
            if (propertyDescriptor.isKey())
            {
                m_keyPropertyDescriptors.emplace_back(propertyDescriptor);
            }
        }
    }

    bool Container<type::Struct>::key_equal::operator()(const type::Struct& lhs, const type::Struct& rhs) const
    {
        const type::PropertyArea& lhsArea = lhs._propertyArea();
        const type::PropertyArea& rhsArea = rhs._propertyArea();

        for (const auto& propertyDescriptor_ : m_keyPropertyDescriptors)
        {
            const type::PropertyDescriptor& pd = propertyDescriptor_.get();
            const auto& lhsValue = lhsArea.getProperty<type::Typeless>(pd.offset());
            const auto& rhsValue = rhsArea.getProperty<type::Typeless>(pd.offset());

            if (!pd.valueDescriptor().equal(lhsValue, rhsValue))
            {
                return false;
            }
        }

        return true;
    }

    bool Container<type::Struct>::key_equal::operator()(const type::AnyStruct& lhs, const type::Struct& rhs) const
    {
        return (*this)(static_cast<const type::Struct&>(lhs), rhs);
    }

    bool Container<type::Struct>::key_equal::operator()(const type::Struct& lhs, const type::AnyStruct& rhs) const
    {
        return (*this)(lhs, static_cast<const type::Struct&>(rhs));
    }

    bool Container<type::Struct>::key_equal::operator()(const type::AnyStruct& lhs, const type::AnyStruct& rhs) const
    {
        return (*this)(static_cast<const type::Struct&>(lhs), static_cast<const type::Struct&>(rhs));
    }

    Container<type::Struct>::Container(const type::StructDescriptor& descriptor) :
        m_descriptor(&descriptor),
        m_instances{ 0, key_hash{ descriptor }, key_equal{ descriptor } }
    {
        for (const type::PropertyDescriptor& propertyDescriptor : descriptor.propertyDescriptors())
        {
            if (!propertyDescriptor.isKey())
            {
                m_noKeyPropertyDescriptors.emplace_back(propertyDescriptor);
            }
        }
    }

    const type::StructDescriptor& Container<type::Struct>::descriptor() const &
    {
        return *m_descriptor;
    }

    auto Container<type::Struct>::begin() const & -> const_iterator_t
    {
        return m_instances.begin();
    }

    auto Container<type::Struct>::end() const & -> const_iterator_t
    {
        return m_instances.end();
    }

    auto Container<type::Struct>::cbegin() const & -> const_iterator_t
    {
        return m_instances.cbegin();
    }

    auto Container<type::Struct>::cend() const & -> const_iterator_t
    {
        return m_instances.cend();
    }

    bool Container<type::Struct>::empty() const &
    {
        return m_instances.empty();
    }

    size_t Container<type::Struct>::size() const &
    {
        return m_instances.size();
    }

    auto Container<type::Struct>::findClone(const type::Struct& instance) const & -> const value_t*
    {
        auto it = m_instances.find(instance);
        return it == m_instances.end() ? nullptr : &*it;
    }

    auto Container<type::Struct>::getClone(const type::Struct& instance) const & -> const value_t &
    {
        const value_t* clone = findClone(instance);

        if (clone == nullptr)
        {
            throw std::logic_error{ "instance is not part of container" };
        }

        return *clone;
    }

    const type::Struct* Container<type::Struct>::find(const type::Struct& instance) const &
    {
        const value_t* clone = findClone(instance);
        return clone == nullptr ? nullptr : &clone->first.get();
    }

    const type::Struct& Container<type::Struct>::get(const type::Struct& instance) const &
    {
        return getClone(instance).first;
    }

    auto Container<type::Struct>::insert(const DotsHeader& header, const type::Struct& instance) & -> const value_t &
    {
        auto it = m_instances.find(instance);

        if (it == m_instances.end())
        {
            auto [itCreated, _] = m_instances.emplace(std::piecewise_construct,
                std::forward_as_tuple(instance),
                std::forward_as_tuple(DotsCloneInformation{
                    .lastOperation = DotsMt::create,
                    .lastUpdateFrom = header.sender,
                    .created = header.sentTime,
                    .createdFrom = header.sender,
                    .modified = header.sentTime,
                    .localUpdateTime = timepoint_t::Now()
                }));

            return *itCreated;
        }
        else
        {
            // Key fields are not touched by updateWithoutKeys, so the hash stays
            // stable and we can mutate the entry in place without re-insertion.
            type::Struct& existing = const_cast<type::AnyStruct&>(it->first).get();
            DotsCloneInformation& cloneInfo = it->second;

            updateWithoutKeys(existing, instance, *header.attributes);
            cloneInfo.lastOperation = DotsMt::update;
            cloneInfo.lastUpdateFrom = header.sender;
            cloneInfo.modified = header.sentTime;
            cloneInfo.localUpdateTime = timepoint_t::Now();

            return *it;
        }
    }

    auto Container<type::Struct>::remove(const DotsHeader& header, const type::Struct& instance) & -> node_t
    {
        auto it = m_instances.find(instance);

        if (it == m_instances.end())
        {
            return node_t{};
        }

        node_t node = m_instances.extract(it);
        type::Struct& removed = node.key();
        DotsCloneInformation& cloneInfo = node.mapped();

        updateWithoutKeys(removed, instance, *header.attributes);
        cloneInfo.lastOperation = DotsMt::remove;
        cloneInfo.lastUpdateFrom = header.sender;
        cloneInfo.modified = header.sentTime;
        cloneInfo.localUpdateTime = timepoint_t::Now();

        return node;
    }

    void Container<type::Struct>::clear() &
    {
        m_instances.clear();
    }

    void Container<type::Struct>::forEachClone(const std::function<void(const value_t&)>& f) const &
    {
        std::for_each(m_instances.begin(), m_instances.end(), f);
    }

    void Container<type::Struct>::forEach(const std::function<void(const type::Struct&)>& f) const &
    {
        forEachClone([&](const value_t& value)
        {
            f(value.first);
        });
    }

    size_t Container<type::Struct>::totalMemoryUsage() const &
    {
        size_t staticMemUsage = sizeof(Container<type::Struct>);
        size_t dynElementMemUsage = m_instances.size() * sizeof(value_t);
        size_t dynInstanceMemUsage = std::accumulate(m_instances.begin(), m_instances.end(), size_t{ 0 }, [](size_t size, const value_t& value)
        {
            return size + value.first->_totalMemoryUsage();
        });

        return staticMemUsage + dynElementMemUsage + dynInstanceMemUsage;
    }

    void Container<type::Struct>::updateWithoutKeys(type::Struct& lhs, const type::Struct& rhs, property_set_t includedSet)
    {
        using namespace type;

        property_set_t updateSet = (lhs._validProperties() + rhs._validProperties()) ^ includedSet;

        PropertyArea& lhsArea = lhs._propertyArea();
        const PropertyArea& rhsArea = rhs._propertyArea();

        property_set_t& lhsValidSet = lhsArea.validProperties();
        property_set_t rhsValidSet = rhsArea.validProperties();

        for (const auto& propertyDescriptor_ : m_noKeyPropertyDescriptors)
        {
            const PropertyDescriptor& propertyDescriptor = propertyDescriptor_.get();

            if (property_set_t propertySet = propertyDescriptor.set(); propertySet <= updateSet)
            {
                const Descriptor<>& valueDescriptor = propertyDescriptor.valueDescriptor();
                auto& lhsValue = lhsArea.getProperty<Typeless>(propertyDescriptor.offset());

                if (propertySet <= rhsValidSet)
                {
                    const auto& rhsValue = rhsArea.getProperty<Typeless>(propertyDescriptor.offset());

                    if (propertySet <= lhsValidSet)
                    {
                        valueDescriptor.assign(lhsValue, rhsValue);
                    }
                    else
                    {
                        valueDescriptor.constructInPlace(lhsValue, rhsValue);
                        lhsValidSet += propertySet;
                    }
                }
                else
                {
                    valueDescriptor.destruct(lhsValue);
                    lhsValidSet -= propertySet;
                }
            }
        }
    }
}
