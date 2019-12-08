#include <dots/type/ProxyPropertyIterator.h>

namespace dots::type
{
	template <bool IsReverse, bool IsConst>
	ProxyPropertyIterator<IsReverse, IsConst>::ProxyPropertyIterator(area_t& area, const descriptor_container_t& descriptors, descriptor_iterator_t descriptorIt, const PropertySet& properties/* = PropertySet::All*/) :
		m_area(&area),
		m_descriptors(&descriptors),
		m_descriptorIt(std::move(descriptorIt)),
		m_properties(properties)
	{
		if (!onEnd() && !emplaceProxy().isPartOf(m_properties))
		{
			++(*this);
		}
	}

	template <bool IsReverse, bool IsConst>
	void ProxyPropertyIterator<IsReverse, IsConst>::swap(ProxyPropertyIterator& other) noexcept
	{
		std::swap(m_area, other.m_area);
		std::swap(m_descriptorIt, other.m_descriptorIt);
	}

	template <bool IsReverse, bool IsConst>
	void ProxyPropertyIterator<IsReverse, IsConst>::swap(ProxyPropertyIterator&& other)
	{
		m_area = other.m_area;
		other.m_area = nullptr;
		m_descriptorIt = std::move(other.m_descriptorIt);
	}

	template <bool IsReverse, bool IsConst>
	auto ProxyPropertyIterator<IsReverse, IsConst>::operator ++ () -> ProxyPropertyIterator&
	{
		while (!onEnd())
		{
			++m_descriptorIt;

			if (onEnd() || emplaceProxy().isPartOf(m_properties))
			{
				break;
			}
		}

		return *this;
	}

	template <bool IsReverse, bool IsConst>
	auto ProxyPropertyIterator<IsReverse, IsConst>::operator -- () -> ProxyPropertyIterator&
	{
		while (!onBegin())
		{
			--m_descriptorIt;

			if (emplaceProxy().isPartOf(m_properties))
			{
				break;
			}
		}

		return *this;
	}

	template <bool IsReverse, bool IsConst>
	auto ProxyPropertyIterator<IsReverse, IsConst>::operator ++ (int) -> ProxyPropertyIterator
	{
		ProxyPropertyIterator copy = *this;
		++(*this);

		return copy;
	}

	template <bool IsReverse, bool IsConst>
	auto ProxyPropertyIterator<IsReverse, IsConst>::operator -- (int) -> ProxyPropertyIterator
	{
		ProxyPropertyIterator copy = *this;
		--(*this);

		return copy;
	}

	template <bool IsReverse, bool IsConst>
	auto ProxyPropertyIterator<IsReverse, IsConst>::operator * () -> reference
	{
		if (onEnd())
		{
			throw std::logic_error{ "attempt to access past-end property" };
		}

		return *m_proxy;
	}

	template <bool IsReverse, bool IsConst>
	auto ProxyPropertyIterator<IsReverse, IsConst>::operator * () const -> const reference
	{
		return *const_cast<ProxyPropertyIterator&>(*this);
	}

	template <bool IsReverse, bool IsConst>
	auto ProxyPropertyIterator<IsReverse, IsConst>::operator -> () -> pointer
	{
		return &*(*this);
	}

	template <bool IsReverse, bool IsConst>
	auto ProxyPropertyIterator<IsReverse, IsConst>::operator -> () const -> const pointer
	{
		return &*(*this);
	}

	template <bool IsReverse, bool IsConst>
	bool ProxyPropertyIterator<IsReverse, IsConst>::operator == (const ProxyPropertyIterator& other) const
	{
		return m_descriptorIt == other.m_descriptorIt;
	}

	template <bool IsReverse, bool IsConst>
	bool ProxyPropertyIterator<IsReverse, IsConst>::operator != (const ProxyPropertyIterator& other) const
	{
		return !(*this == other);
	}

	template <bool IsReverse, bool IsConst>
	bool ProxyPropertyIterator<IsReverse, IsConst>::onBegin() const
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
	bool ProxyPropertyIterator<IsReverse, IsConst>::onEnd() const
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
	auto ProxyPropertyIterator<IsReverse, IsConst>::emplaceProxy() -> reference
	{
		if constexpr (IsConst)
		{
			return m_proxy.emplace(const_cast<std::remove_const_t<area_t>&>(*m_area), *m_descriptorIt);
		}
		else
		{
			return m_proxy.emplace(*m_area, *m_descriptorIt);
		}
	}

	template struct ProxyPropertyIterator<false, false>;
	template struct ProxyPropertyIterator<false, true>;
	template struct ProxyPropertyIterator<true, false>;
	template struct ProxyPropertyIterator<true, true>;

	template <typename Iterator>
	ProxyPropertyRange<Iterator>::ProxyPropertyRange(Iterator begin, Iterator end):
		m_begin(std::move(begin)),
		m_end(std::move(end))
	{
		/* do nothing */
	}

	template <typename Iterator>
	Iterator ProxyPropertyRange<Iterator>::begin() const
	{
		return m_begin;
	}

	template <typename Iterator>
	Iterator ProxyPropertyRange<Iterator>::end() const
	{
		return m_end;
	}

	template struct ProxyPropertyRange<proxy_property_iterator>;
	template struct ProxyPropertyRange<const_proxy_property_iterator>;
	template struct ProxyPropertyRange<reverse_proxy_property_iterator>;
	template struct ProxyPropertyRange<const_reverse_proxy_property_iterator>;
}