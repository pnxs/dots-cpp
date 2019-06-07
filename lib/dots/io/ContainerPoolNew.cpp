#include <dots/io/ContainerPoolNew.h>
#include <algorithm>
#include <utility>
#include <numeric>

namespace dots
{
    auto ContainerPoolNew::begin() const -> const_iterator_t
    {
        return m_pool.begin();
    }

    auto ContainerPoolNew::end() const -> const_iterator_t
    {
        return m_pool.end();
    }

    ContainerPoolNew::iterator_t ContainerPoolNew::begin()
    {
		return m_pool.begin();
    }

    ContainerPoolNew::iterator_t ContainerPoolNew::end()
    {
		return m_pool.end();
    }

    auto ContainerPoolNew::cbegin() const -> const_iterator_t
    {
        return m_pool.cbegin();
    }

    auto ContainerPoolNew::cend() const -> const_iterator_t
    {
        return m_pool.cend();
    }

    bool ContainerPoolNew::empty() const
    {
	    return m_pool.empty();
    }

    size_t ContainerPoolNew::size() const
    {
	    return m_pool.size();
    }

    const ContainerNew<>* ContainerPoolNew::find(const type::StructDescriptor& descriptor) const
    {
		auto it = m_pool.find(&descriptor);
		return it == m_pool.end() ? nullptr : &it->second;
    }

    const ContainerNew<>& ContainerPoolNew::get(const type::StructDescriptor& descriptor) const
    {
		const ContainerNew<>* container = find(descriptor);

		if (container == nullptr)
		{
			throw std::runtime_error{ "container pool does not contain an element for the given type: " + descriptor.name() };
		}

		return *container;
    }

    ContainerNew<>* ContainerPoolNew::find(const type::StructDescriptor& descriptor)
    {
		return const_cast<ContainerNew<>*>(std::as_const(*this).find(descriptor));
    }

    ContainerNew<>& ContainerPoolNew::get(const type::StructDescriptor& descriptor, bool insertIfNotExist/* = true*/)
    {
        if (insertIfNotExist)
        {
            auto [it, emplaced] = m_pool.try_emplace(&descriptor, descriptor);
			auto& [descriptor, container] = *it;

			if (emplaced)
			{
				m_nameCache.emplace(descriptor->name(), &container);
			}

			return container;
        }
        else
        {
			return const_cast<ContainerNew<>&>(std::as_const(*this).get(descriptor));
        }        
    }

	const ContainerNew<>* ContainerPoolNew::find(const std::string_view& name) const
    {
	    auto it = m_nameCache.find(name);
		return it == m_nameCache.end() ? nullptr : it->second;
    }

    const ContainerNew<>& ContainerPoolNew::get(const std::string_view& name) const
    {
	    const ContainerNew<>* container = find(name);

		if (container == nullptr)
		{
			throw std::runtime_error{ "container pool does not contain an element for the given type: " + std::string{ name } };
		}

		return *container;
    }

	ContainerNew<>* ContainerPoolNew::find(const std::string_view& name)
    {
	    return const_cast<ContainerNew<>*>(std::as_const(*this).find(name));
    }

	ContainerNew<>& ContainerPoolNew::get(const std::string_view& name)
    {
	    return const_cast<ContainerNew<>&>(std::as_const(*this).get(name));
    }

    auto ContainerPoolNew::remove(const type::StructDescriptor& descriptor) -> node_t
    {
        node_t node = m_pool.extract(&descriptor);

        if (node.empty())
        {
            throw std::runtime_error{ "container pool does not contain an element for the given type: " + descriptor.name() };
        }

        return node;
    }

    void ContainerPoolNew::forEach(const std::function<void(const ContainerNew<>&)>& f) const
    {
        std::for_each(m_pool.begin(), m_pool.end(), [&](const value_t& value)
        {
            f(value.second);
        });
    }

    size_t ContainerPoolNew::totalMemoryUsage() const
    {
	    return std::accumulate(m_pool.begin(), m_pool.end(), 0u, [](size_t size, const value_t& value)
	    {
		    return size + value.second.totalMemoryUsage();
	    });
    }
}