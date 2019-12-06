#include <dots/io/Container.h>
#include <algorithm>
#include <numeric>

namespace dots
{
	Container<type::NewStruct>::Container(const type::NewStructDescriptor<>& descriptor) :
		m_descriptor(&descriptor)
	{
		/* do nothing */
	}

	const type::NewStructDescriptor<>& Container<type::NewStruct>::descriptor() const
	{
		return *m_descriptor;
	}

	auto Container<type::NewStruct>::begin() const -> const_iterator_t
	{
		return m_instances.begin();
	}

	auto Container<type::NewStruct>::end() const -> const_iterator_t
	{
		return m_instances.end();
	}

	auto Container<type::NewStruct>::cbegin() const -> const_iterator_t
	{
		return m_instances.cbegin();
	}

	auto Container<type::NewStruct>::cend() const -> const_iterator_t
	{
		return m_instances.cend();
	}

	bool Container<type::NewStruct>::empty() const
	{
		return m_instances.empty();
	}

	size_t Container<type::NewStruct>::size() const
	{
		return m_instances.size();
	}

	auto Container<type::NewStruct>::findClone(const type::NewStruct& instance) const -> const value_t*
	{
		auto it = m_instances.find(instance);
		return it == m_instances.end() ? nullptr : &*it;
	}

	auto Container<type::NewStruct>::getClone(const type::NewStruct& instance) const -> const value_t &
	{
		const value_t* clone = findClone(instance);

		if (clone == nullptr)
		{
			throw std::logic_error{ "instance is not part of container" };
		}

		return *clone;
	}

	const type::NewStruct* Container<type::NewStruct>::find(const type::NewStruct& instance) const
	{
		const value_t* clone = findClone(instance);
		return clone == nullptr ? nullptr : &clone->first.get();
	}

	const type::NewStruct& Container<type::NewStruct>::get(const type::NewStruct& instance) const
	{
		return getClone(instance).first;
	}

	auto Container<type::NewStruct>::insert(const DotsHeader& header, const type::NewStruct& instance) -> const value_t &
	{
		auto [itLower, itUpper] = m_instances.equal_range(instance);
		bool unknownInstance = itLower == itUpper;

		if (unknownInstance)
		{
			auto itInserted = m_instances.emplace_hint(itUpper, instance, DotsCloneInformation{
				DotsCloneInformation::lastOperation_i{ DotsMt::create },
				DotsCloneInformation::createdFrom_i{ header.sender },
				DotsCloneInformation::created_i{ header.sentTime },
				DotsCloneInformation::localUpdateTime_i{ pnxs::SystemNow{} }
				});

			return *itInserted;
		}
		else
		{
			node_t node = m_instances.extract(itLower);
			type::NewStruct& existing = node.key();
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

	auto Container<type::NewStruct>::remove(const DotsHeader& header, const type::NewStruct& instance) -> node_t
	{
		node_t node = m_instances.extract(instance);

		if (node.empty())
		{
			throw std::logic_error{ "instance to remove is not part of container" };
		}

		type::NewStruct& removed = node.key();
		DotsCloneInformation& cloneInfo = node.mapped();

		removed._copy(instance, instance._validProperties() - instance._keyProperties());
		cloneInfo.lastOperation = DotsMt::remove;
		cloneInfo.lastUpdateFrom = header.sender;
		cloneInfo.modified = header.sentTime;
		cloneInfo.localUpdateTime = pnxs::SystemNow{};

		return node;
	}

	void Container<type::NewStruct>::clear()
	{
		m_instances.clear();
	}

	void Container<type::NewStruct>::forEachClone(const std::function<void(const value_t&)>& f) const
	{
		std::for_each(m_instances.begin(), m_instances.end(), f);
	}

	void Container<type::NewStruct>::forEach(const std::function<void(const type::NewStruct&)>& f) const
	{
		forEachClone([&](const value_t& value)
		{
			f(value.first);
		});
	}

	size_t Container<type::NewStruct>::totalMemoryUsage() const
	{
		size_t staticMemUsage = sizeof(Container<type::NewStruct>);
		size_t dynElementMemUsage = m_instances.size() * sizeof(value_t);
		size_t dynInstanceMemUsage = std::accumulate(m_instances.begin(), m_instances.end(), 0u, [](size_t size, const value_t& value)
		{
			return size + value.first->_totalMemoryUsage();
		});

		return staticMemUsage + dynElementMemUsage + dynInstanceMemUsage;
	}
}