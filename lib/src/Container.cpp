// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/Container.h>
#include <algorithm>
#include <numeric>

namespace dots
{
    Container<type::Struct>::key_compare::key_compare(const type::StructDescriptor& descriptor)
    {
        for (const type::PropertyDescriptor& propertyDescriptor : descriptor.propertyDescriptors())
        {
            if (propertyDescriptor.isKey())
            {
                m_keyPropertyDescriptors.emplace_back(propertyDescriptor);
            }
        }
    }

    bool Container<type::Struct>::key_compare::operator()(const type::Struct& lhs, const type::Struct& rhs) const
    {
        const type::PropertyArea& lhsPropertyArea = lhs._propertyArea();
        const type::PropertyArea& rhsPropertyArea = rhs._propertyArea();

        for (const auto& propertyDescriptor_ : m_keyPropertyDescriptors)
        {
            const type::PropertyDescriptor& propertyDescriptor = propertyDescriptor_.get();
            const auto& lhsValue = lhsPropertyArea.getProperty<type::Typeless>(propertyDescriptor.offset());
            const auto& rhsValue = rhsPropertyArea.getProperty<type::Typeless>(propertyDescriptor.offset());

            const type::Descriptor<>& valueDescriptor = propertyDescriptor.valueDescriptor();

            if (valueDescriptor.less(lhsValue, rhsValue))
            {
                return true;
            }
            else if (valueDescriptor.less(rhsValue, lhsValue))
            {
                return false;
            }
        }

        return false;
    }

    bool Container<type::Struct>::key_compare::operator()(const type::AnyStruct& lhs, const type::Struct& rhs) const
    {
        return (*this)(static_cast<const type::Struct&>(lhs), rhs);
    }

    bool Container<type::Struct>::key_compare::operator()(const type::Struct& lhs, const type::AnyStruct& rhs) const
    {
        return (*this)(lhs, static_cast<const type::Struct&>(rhs));
    }

    bool Container<type::Struct>::key_compare::operator()(const type::AnyStruct& lhs, const type::AnyStruct& rhs) const
    {
        return (*this)(static_cast<const type::Struct&>(lhs), static_cast<const type::Struct&>(rhs));
    }

    Container<type::Struct>::Container(const type::StructDescriptor& descriptor) :
        m_descriptor(&descriptor),
        m_instances{ descriptor }
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
        auto [itLower, itUpper] = m_instances.equal_range(instance);
        bool unknownInstance = itLower == itUpper;

        if (unknownInstance)
        {
            auto itCreated = m_instances.emplace_hint(itUpper, instance, DotsCloneInformation{
                .lastOperation = DotsMt::create,
                .lastUpdateFrom = header.sender,
                .created = header.sentTime,
                .createdFrom = header.sender,
                .modified = header.sentTime,
                .localUpdateTime = timepoint_t::Now()
            });

            return *itCreated;
        }
        else
        {
            node_t node = m_instances.extract(itLower);
            type::Struct& existing = node.key();
            DotsCloneInformation& cloneInfo = node.mapped();

            updateWithoutKeys(existing, instance, *header.attributes);
            cloneInfo.lastOperation = DotsMt::update;
            cloneInfo.lastUpdateFrom = header.sender;
            cloneInfo.modified = header.sentTime;
            cloneInfo.localUpdateTime = timepoint_t::Now();

            auto itUpdated = m_instances.insert(itUpper, std::move(node));

            return *itUpdated;
        }
    }

    auto Container<type::Struct>::remove(const DotsHeader& header, const type::Struct& instance) & -> node_t
    {
        node_t node = m_instances.extract(instance);

        if (!node.empty())
        {
            type::Struct& removed = node.key();
            DotsCloneInformation& cloneInfo = node.mapped();

            updateWithoutKeys(removed, instance, *header.attributes);
            cloneInfo.lastOperation = DotsMt::remove;
            cloneInfo.lastUpdateFrom = header.sender;
            cloneInfo.modified = header.sentTime;
            cloneInfo.localUpdateTime = timepoint_t::Now();
        }

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
