#include <dots/io/ContainerPool.h>
#include <algorithm>
#include <utility>
#include <numeric>

namespace dots::io
{
    auto ContainerPool::begin() const -> const_iterator_t
    {
        return m_pool.begin();
    }

    auto ContainerPool::end() const -> const_iterator_t
    {
        return m_pool.end();
    }

    ContainerPool::iterator_t ContainerPool::begin()
    {
        return m_pool.begin();
    }

    ContainerPool::iterator_t ContainerPool::end()
    {
        return m_pool.end();
    }

    auto ContainerPool::cbegin() const -> const_iterator_t
    {
        return m_pool.cbegin();
    }

    auto ContainerPool::cend() const -> const_iterator_t
    {
        return m_pool.cend();
    }

    bool ContainerPool::empty() const
    {
        return m_pool.empty();
    }

    size_t ContainerPool::size() const
    {
        return m_pool.size();
    }

    const Container<>* ContainerPool::find(const type::StructDescriptor<>& descriptor) const
    {
        auto it = m_pool.find(&descriptor);
        return it == m_pool.end() ? nullptr : &it->second;
    }

    const Container<>& ContainerPool::get(const type::StructDescriptor<>& descriptor, bool insertIfNotExist/* = true*/) const
    {
        if (insertIfNotExist)
        {
            auto [it, emplaced] = m_pool.try_emplace(&descriptor, descriptor);
            auto& [descriptorPtr, container] = *it;

            if (emplaced)
            {
                m_nameCache.emplace(descriptorPtr->name(), &container);
            }

            return container;
        }
        else
        {
            const Container<>* container = find(descriptor);

            if (container == nullptr)
            {
                throw std::runtime_error{ "container pool does not contain an element for the given type: " + descriptor.name() };
            }

            return *container;
        }
    }

    Container<>* ContainerPool::find(const type::StructDescriptor<>& descriptor)
    {
        return const_cast<Container<>*>(std::as_const(*this).find(descriptor));
    }

    Container<>& ContainerPool::get(const type::StructDescriptor<>& descriptor, bool insertIfNotExist/* = true*/)
    {
        return const_cast<Container<>&>(std::as_const(*this).get(descriptor, insertIfNotExist));
    }

    const Container<>* ContainerPool::find(const std::string_view& name) const
    {
        auto it = m_nameCache.find(name);
        return it == m_nameCache.end() ? nullptr : it->second;
    }

    const Container<>& ContainerPool::get(const std::string_view& name) const
    {
        const Container<>* container = find(name);

        if (container == nullptr)
        {
            throw std::runtime_error{ "container pool does not contain an element for the given type: " + std::string{ name } };
        }

        return *container;
    }

    Container<>* ContainerPool::find(const std::string_view& name)
    {
        return const_cast<Container<>*>(std::as_const(*this).find(name));
    }

    Container<>& ContainerPool::get(const std::string_view& name)
    {
        return const_cast<Container<>&>(std::as_const(*this).get(name));
    }

    auto ContainerPool::remove(const type::StructDescriptor<>& descriptor) -> node_t
    {
        node_t node = m_pool.extract(&descriptor);

        if (node.empty())
        {
            throw std::runtime_error{ "container pool does not contain an element for the given type: " + descriptor.name() };
        }

        return node;
    }

    void ContainerPool::forEach(const std::function<void(const Container<>&)>& f) const
    {
        std::for_each(m_pool.begin(), m_pool.end(), [&](const value_t& value)
        {
            f(value.second);
        });
    }

    size_t ContainerPool::totalMemoryUsage() const
    {
        return std::accumulate(m_pool.begin(), m_pool.end(), size_t{ 0 }, [](size_t size, const value_t& value)
        {
            return size + value.second.totalMemoryUsage();
        });
    }
}