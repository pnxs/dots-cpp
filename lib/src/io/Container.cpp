#include <dots/io/Container.h>
#include <algorithm>
#include <numeric>

namespace dots::io
{
    Container<type::Struct>::Container(const type::StructDescriptor<>& descriptor) :
        m_descriptor(&descriptor)
    {
        /* do nothing */
    }

    const type::StructDescriptor<>& Container<type::Struct>::descriptor() const
    {
        return *m_descriptor;
    }

    auto Container<type::Struct>::begin() const -> const_iterator_t
    {
        return m_instances.begin();
    }

    auto Container<type::Struct>::end() const -> const_iterator_t
    {
        return m_instances.end();
    }

    auto Container<type::Struct>::cbegin() const -> const_iterator_t
    {
        return m_instances.cbegin();
    }

    auto Container<type::Struct>::cend() const -> const_iterator_t
    {
        return m_instances.cend();
    }

    bool Container<type::Struct>::empty() const
    {
        return m_instances.empty();
    }

    size_t Container<type::Struct>::size() const
    {
        return m_instances.size();
    }

    auto Container<type::Struct>::findClone(const type::Struct& instance) const -> const value_t*
    {
        auto it = m_instances.find(instance);
        return it == m_instances.end() ? nullptr : &*it;
    }

    auto Container<type::Struct>::getClone(const type::Struct& instance) const -> const value_t &
    {
        const value_t* clone = findClone(instance);

        if (clone == nullptr)
        {
            throw std::logic_error{ "instance is not part of container" };
        }

        return *clone;
    }

    const type::Struct* Container<type::Struct>::find(const type::Struct& instance) const
    {
        const value_t* clone = findClone(instance);
        return clone == nullptr ? nullptr : &clone->first.get();
    }

    const type::Struct& Container<type::Struct>::get(const type::Struct& instance) const
    {
        return getClone(instance).first;
    }

    auto Container<type::Struct>::insert(const DotsHeader& header, const type::Struct& instance) -> const value_t &
    {
        auto [itLower, itUpper] = m_instances.equal_range(instance);
        bool unknownInstance = itLower == itUpper;

        if (unknownInstance)
        {
            auto itInserted = m_instances.emplace_hint(itUpper, instance, DotsCloneInformation{
                DotsCloneInformation::lastOperation_i{ DotsMt::create },
                DotsCloneInformation::lastUpdateFrom_i{ header.sender },
                DotsCloneInformation::created_i{ header.sentTime },
                DotsCloneInformation::createdFrom_i{ header.sender },
                DotsCloneInformation::modified_i{ header.sentTime },
                DotsCloneInformation::localUpdateTime_i{ types::timepoint_t::Now() }
            });

            return *itInserted;
        }
        else
        {
            node_t node = m_instances.extract(itLower);
            type::Struct& existing = node.key();
            DotsCloneInformation& cloneInfo = node.mapped();

            existing._copy(instance, *header.attributes - instance._keyProperties());
            cloneInfo.lastOperation = DotsMt::update;
            cloneInfo.lastUpdateFrom = header.sender;
            cloneInfo.modified = header.sentTime;
            cloneInfo.localUpdateTime = types::timepoint_t::Now();

            auto itUpdated = m_instances.insert(itUpper, std::move(node));

            return *itUpdated;
        }
    }

    auto Container<type::Struct>::remove(const DotsHeader& header, const type::Struct& instance) -> node_t
    {
        node_t node = m_instances.extract(instance);

        if (!node.empty())
        {
            type::Struct& removed = node.key();
            DotsCloneInformation& cloneInfo = node.mapped();

            removed._copy(instance, *header.attributes - instance._keyProperties());
            cloneInfo.lastOperation = DotsMt::remove;
            cloneInfo.lastUpdateFrom = header.sender;
            cloneInfo.modified = header.sentTime;
            cloneInfo.localUpdateTime = types::timepoint_t::Now();
        }

        return node;
    }

    void Container<type::Struct>::clear()
    {
        m_instances.clear();
    }

    void Container<type::Struct>::forEachClone(const std::function<void(const value_t&)>& f) const
    {
        std::for_each(m_instances.begin(), m_instances.end(), f);
    }

    void Container<type::Struct>::forEach(const std::function<void(const type::Struct&)>& f) const
    {
        forEachClone([&](const value_t& value)
        {
            f(value.first);
        });
    }

    size_t Container<type::Struct>::totalMemoryUsage() const
    {
        size_t staticMemUsage = sizeof(Container<type::Struct>);
        size_t dynElementMemUsage = m_instances.size() * sizeof(value_t);
        size_t dynInstanceMemUsage = std::accumulate(m_instances.begin(), m_instances.end(), 0u, [](size_t size, const value_t& value)
        {
            return size + value.first->_totalMemoryUsage();
        });

        return staticMemUsage + dynElementMemUsage + dynInstanceMemUsage;
    }
}