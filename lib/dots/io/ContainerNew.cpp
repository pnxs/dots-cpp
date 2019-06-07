#include <dots/io/ContainerNew.h>
#include <algorithm>
#include <numeric>

namespace dots
{
	ContainerNew<type::Struct>::ContainerNew(const type::StructDescriptor& descriptor) :
		m_descriptor(&descriptor)
	{
		/* do nothing */
	}

	const type::StructDescriptor& ContainerNew<type::Struct>::descriptor() const
	{
		return *m_descriptor;
	}

	auto ContainerNew<type::Struct>::begin() const -> const_iterator_t
	{
		return m_instances.begin();
	}

	auto ContainerNew<type::Struct>::end() const -> const_iterator_t
	{
		return m_instances.end();
	}

	auto ContainerNew<type::Struct>::cbegin() const -> const_iterator_t
	{
		return m_instances.cbegin();
	}

	auto ContainerNew<type::Struct>::cend() const -> const_iterator_t
	{
		return m_instances.cend();
	}

	bool ContainerNew<type::Struct>::empty() const
	{
		return m_instances.empty();
	}

	size_t ContainerNew<type::Struct>::size() const
	{
		return m_instances.size();
	}

	auto ContainerNew<type::Struct>::findClone(const type::Struct& instance) const -> const value_t*
	{
		auto it = m_instances.find(instance);
		return it == m_instances.end() ? nullptr : &*it;
	}

	auto ContainerNew<type::Struct>::getClone(const type::Struct& instance) const -> const value_t &
	{
		const value_t* clone = findClone(instance);

		if (clone == nullptr)
		{
			throw std::logic_error{ "instance is not part of container" };
		}

		return *clone;
	}

	const type::Struct* ContainerNew<type::Struct>::find(const type::Struct& instance) const
	{
		const value_t* clone = findClone(instance);
		return clone == nullptr ? nullptr : &clone->first.get();
	}

	const type::Struct& ContainerNew<type::Struct>::get(const type::Struct& instance) const
	{
		return getClone(instance).first;
	}

	auto ContainerNew<type::Struct>::insert(const DotsHeader& header, const type::Struct& instance) -> const value_t &
	{
		auto [itLower, itUpper] = m_instances.equal_range(instance);
		bool unknownInstance = itLower == itUpper;

		if (unknownInstance)
		{
			auto itInserted = m_instances.emplace_hint(itUpper, instance, DotsCloneInformation{
				DotsCloneInformation::lastOperation_t_i{ DotsMt::create },
				DotsCloneInformation::createdFrom_t_i{ header.sender },
				DotsCloneInformation::created_t_i{ header.sentTime },
				DotsCloneInformation::localUpdateTime_t_i{ pnxs::SystemNow{} }
				});

			return *itInserted;
		}
		else
		{
			node_t node = m_instances.extract(itLower);
			type::Struct& existing = node.key();
			DotsCloneInformation& cloneInfo = node.mapped();

			existing._copy(instance, instance._validProperties() - instance._keyProperties());
			cloneInfo.lastOperation = DotsMt::update;
			cloneInfo.lastUpdateFrom = header.sender;
			cloneInfo.modified = header.sentTime;
			cloneInfo.localUpdateTime = pnxs::SystemNow{};

			auto itUpdated = m_instances.insert(itUpper, std::move(node));

			return *itUpdated;
		}
	}

	auto ContainerNew<type::Struct>::remove(const DotsHeader& header, const type::Struct& instance) -> node_t
	{
		node_t node = m_instances.extract(instance);

		if (node.empty())
		{
			throw std::logic_error{ "instance to remove is not part of container" };
		}

		type::Struct& removed = node.key();
		DotsCloneInformation& cloneInfo = node.mapped();

		removed._copy(instance, instance._validProperties() - instance._keyProperties());
		cloneInfo.lastOperation = DotsMt::remove;
		cloneInfo.lastUpdateFrom = header.sender;
		cloneInfo.modified = header.sentTime;
		cloneInfo.localUpdateTime = pnxs::SystemNow{};

		return node;
	}

	void ContainerNew<type::Struct>::clear()
	{
		m_instances.clear();
	}

	void ContainerNew<type::Struct>::forEachClone(const std::function<void(const value_t&)>& f) const
	{
		std::for_each(m_instances.begin(), m_instances.end(), f);
	}

	void ContainerNew<type::Struct>::forEach(const std::function<void(const type::Struct&)>& f) const
	{
		forEachClone([&](const value_t& value)
		{
			f(value.first);
		});
	}

	size_t ContainerNew<type::Struct>::totalMemoryUsage() const
	{
		size_t staticMemUsage = sizeof(ContainerNew<type::Struct>);
		size_t dynElementMemUsage = m_instances.size() * sizeof(value_t);
		size_t dynInstanceMemUsage = std::accumulate(m_instances.begin(), m_instances.end(), 0u, [](size_t size, const value_t& value)
		{
			return size + value.first->_totalMemoryUsage();
		});

		return staticMemUsage + dynElementMemUsage + dynInstanceMemUsage;
	}
}