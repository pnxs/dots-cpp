#include <dots/type/NewProxyPropertyIterator.h>

namespace dots::type
{
	template <bool IsReverse, bool IsConst>
	NewProxyPropertyIterator<IsReverse, IsConst>::NewProxyPropertyIterator(area_t& area, const descriptor_container_t& descriptors, descriptor_iterator_t descriptorIt, const NewPropertySet& properties/* = NewPropertySet::All*/) :
		m_area(&area),
		m_descriptors(&descriptors),
		m_descriptorIt(std::move(descriptorIt)),
		m_properties(properties)
	{
		if (!onEnd() && (*m_descriptorIt == nullptr || !emplaceProxy().isPartOf(m_properties)))
		{
			++(*this);
		}
	}

	template <bool IsReverse, bool IsConst>
	void NewProxyPropertyIterator<IsReverse, IsConst>::swap(NewProxyPropertyIterator& other) noexcept
	{
		std::swap(m_area, other.m_area);
		std::swap(m_descriptorIt, other.m_descriptorIt);
	}

	template <bool IsReverse, bool IsConst>
	void NewProxyPropertyIterator<IsReverse, IsConst>::swap(NewProxyPropertyIterator&& other)
	{
		m_area = other.m_area;
		other.m_area = nullptr;
		m_descriptorIt = std::move(other.m_descriptorIt);
	}

	template <bool IsReverse, bool IsConst>
	auto NewProxyPropertyIterator<IsReverse, IsConst>::operator ++ () -> NewProxyPropertyIterator&
	{
		while (!onEnd())
		{
			++m_descriptorIt;

			if (onEnd() || (*m_descriptorIt != nullptr && emplaceProxy().isPartOf(m_properties)))
			{
				break;
			}
		}

		return *this;
	}

	template <bool IsReverse, bool IsConst>
	auto NewProxyPropertyIterator<IsReverse, IsConst>::operator -- () -> NewProxyPropertyIterator&
	{
		while (!onBegin())
		{
			--m_descriptorIt;

			if (*m_descriptorIt != nullptr && emplaceProxy().isPartOf(m_properties))
			{
				break;
			}
		}

		return *this;
	}

	template <bool IsReverse, bool IsConst>
	auto NewProxyPropertyIterator<IsReverse, IsConst>::operator ++ (int) -> NewProxyPropertyIterator
	{
		NewProxyPropertyIterator copy = *this;
		++(*this);

		return copy;
	}

	template <bool IsReverse, bool IsConst>
	auto NewProxyPropertyIterator<IsReverse, IsConst>::operator -- (int) -> NewProxyPropertyIterator
	{
		NewProxyPropertyIterator copy = *this;
		--(*this);

		return copy;
	}

	template <bool IsReverse, bool IsConst>
	auto NewProxyPropertyIterator<IsReverse, IsConst>::operator * () -> reference
	{
		if (onEnd())
		{
			throw std::logic_error{ "attempt to access past-end property" };
		}

		return *m_proxy;
	}

	template <bool IsReverse, bool IsConst>
	auto NewProxyPropertyIterator<IsReverse, IsConst>::operator * () const -> const reference
	{
		return *const_cast<NewProxyPropertyIterator&>(*this);
	}

	template <bool IsReverse, bool IsConst>
	auto NewProxyPropertyIterator<IsReverse, IsConst>::operator -> () -> pointer
	{
		return &*(*this);
	}

	template <bool IsReverse, bool IsConst>
	auto NewProxyPropertyIterator<IsReverse, IsConst>::operator -> () const -> const pointer
	{
		return &*(*this);
	}

	template <bool IsReverse, bool IsConst>
	bool NewProxyPropertyIterator<IsReverse, IsConst>::operator == (const NewProxyPropertyIterator& other) const
	{
		return m_descriptorIt == other.m_descriptorIt;
	}

	template <bool IsReverse, bool IsConst>
	bool NewProxyPropertyIterator<IsReverse, IsConst>::operator != (const NewProxyPropertyIterator& other) const
	{
		return !(*this == other);
	}

	template <bool IsReverse, bool IsConst>
	bool NewProxyPropertyIterator<IsReverse, IsConst>::onBegin() const
	{
		if constexpr (IsReverse)
		{
			return m_descriptorIt == m_descriptors->rbegin();
		}
		else
		{
			return m_descriptorIt == m_descriptors->begin();
		}
	}

	template <bool IsReverse, bool IsConst>
	bool NewProxyPropertyIterator<IsReverse, IsConst>::onEnd() const
	{
		if constexpr (IsReverse)
		{
			return m_descriptorIt == m_descriptors->rend();
		}
		else
		{
			return m_descriptorIt == m_descriptors->end();
		}
	}

	template <bool IsReverse, bool IsConst>
	auto NewProxyPropertyIterator<IsReverse, IsConst>::emplaceProxy() -> reference
	{
		if constexpr (IsConst)
		{
			return m_proxy.emplace(const_cast<std::remove_const_t<area_t>&>(*m_area), **m_descriptorIt);
		}
		else
		{
			return m_proxy.emplace(*m_area, **m_descriptorIt);
		}
	}

	template struct NewProxyPropertyIterator<false, false>;
	template struct NewProxyPropertyIterator<false, true>;
	template struct NewProxyPropertyIterator<true, false>;
	template struct NewProxyPropertyIterator<true, true>;

	template <typename Iterator>
	NewProxyPropertyRange<Iterator>::NewProxyPropertyRange(Iterator begin, Iterator end):
		m_begin(std::move(begin)),
		m_end(std::move(end))
	{
		/* do nothing */
	}

	template <typename Iterator>
	Iterator NewProxyPropertyRange<Iterator>::begin() const
	{
		return m_begin;
	}

	template <typename Iterator>
	Iterator NewProxyPropertyRange<Iterator>::end() const
	{
		return m_end;
	}

	template struct NewProxyPropertyRange<new_proxy_property_iterator>;
	template struct NewProxyPropertyRange<new_const_proxy_property_iterator>;
	template struct NewProxyPropertyRange<new_reverse_proxy_property_iterator>;
	template struct NewProxyPropertyRange<new_const_reverse_proxy_property_iterator>;
}