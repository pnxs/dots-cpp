#include <dots/type/NewProxyPropertyPairIterator.h>

namespace dots::type
{
	template <typename LhsIterator, typename RhsIterator>
	ProxyPropertyPairIterator<LhsIterator, RhsIterator>::ProxyPropertyPairIterator(inner_lhs_iterator_t innerIteratorLhs, inner_rhs_iterator_t innerIteratorRhs) :
		_innerIteratorLhs(std::move(innerIteratorLhs)),
		_innerIteratorRhs(std::move(innerIteratorRhs))
	{
		/* do nothing */
	}

	template <typename LhsIterator, typename RhsIterator>
	void ProxyPropertyPairIterator<LhsIterator, RhsIterator>::swap(ProxyPropertyPairIterator& other) noexcept
	{
		std::swap(_innerIteratorLhs, other._innerIteratorLhs);
		std::swap(_innerIteratorRhs, other._innerIteratorRhs);
	}

	template <typename LhsIterator, typename RhsIterator>
	void ProxyPropertyPairIterator<LhsIterator, RhsIterator>::swap(ProxyPropertyPairIterator&& other)
	{
		_innerIteratorLhs = std::move(other._innerIteratorLhs);
		_innerIteratorRhs = std::move(other._innerIteratorRhs);
	}

	template <typename LhsIterator, typename RhsIterator>
	auto ProxyPropertyPairIterator<LhsIterator, RhsIterator>::operator ++ () -> ProxyPropertyPairIterator&
	{
		++_innerIteratorLhs;
		++_innerIteratorRhs;

		return *this;
	}

	template <typename LhsIterator, typename RhsIterator>
	auto ProxyPropertyPairIterator<LhsIterator, RhsIterator>::operator -- () -> ProxyPropertyPairIterator&
	{
		--_innerIteratorLhs;
		--_innerIteratorRhs;

		return *this;
	}

	template <typename LhsIterator, typename RhsIterator>
	auto ProxyPropertyPairIterator<LhsIterator, RhsIterator>::operator ++ (int) -> ProxyPropertyPairIterator
	{
		return ProxyPropertyPairIterator{ _innerIteratorLhs++, _innerIteratorRhs++ };
	}

	template <typename LhsIterator, typename RhsIterator>
	auto ProxyPropertyPairIterator<LhsIterator, RhsIterator>::operator -- (int) -> ProxyPropertyPairIterator
	{
		return ProxyPropertyPairIterator{ _innerIteratorLhs--, _innerIteratorRhs-- };
	}

	template <typename LhsIterator, typename RhsIterator>
	auto ProxyPropertyPairIterator<LhsIterator, RhsIterator>::operator * () -> reference
	{
		return _proxyPair.emplace(*_innerIteratorLhs, *_innerIteratorRhs);
	}

	template <typename LhsIterator, typename RhsIterator>
	auto ProxyPropertyPairIterator<LhsIterator, RhsIterator>::operator * () const -> const_reference
	{
		return *const_cast<ProxyPropertyPairIterator&>(*this);
	}

	template <typename LhsIterator, typename RhsIterator>
	auto ProxyPropertyPairIterator<LhsIterator, RhsIterator>::operator -> () -> pointer
	{
		return &*(*this);
	}

	template <typename LhsIterator, typename RhsIterator>
	auto ProxyPropertyPairIterator<LhsIterator, RhsIterator>::operator -> () const -> const_pointer
	{
		return &*(*this);
	}

	template <typename LhsIterator, typename RhsIterator>
	bool ProxyPropertyPairIterator<LhsIterator, RhsIterator>::operator == (const ProxyPropertyPairIterator& other) const
	{
		return _innerIteratorLhs == other._innerIteratorLhs && _innerIteratorRhs == other._innerIteratorRhs;
	}

	template <typename LhsIterator, typename RhsIterator>
	bool ProxyPropertyPairIterator<LhsIterator, RhsIterator>::operator != (const ProxyPropertyPairIterator& other) const
	{
		return !(*this == other);
	}

	template struct ProxyPropertyPairIterator<proxy_property_iterator, proxy_property_iterator>;
	template struct ProxyPropertyPairIterator<proxy_property_iterator, const_proxy_property_iterator>;
	template struct ProxyPropertyPairIterator<const_proxy_property_iterator, const_proxy_property_iterator>;
	template struct ProxyPropertyPairIterator<reverse_proxy_property_iterator, reverse_proxy_property_iterator>;
	template struct ProxyPropertyPairIterator<reverse_proxy_property_iterator, const_reverse_proxy_property_iterator>;
	template struct ProxyPropertyPairIterator<const_reverse_proxy_property_iterator, const_reverse_proxy_property_iterator>;

	template <typename Iterator>
	ProxyPropertyPairRange<Iterator>::ProxyPropertyPairRange(Iterator begin, Iterator end):
		_begin(std::move(begin)),
		_end(std::move(end))
	{
		/* do nothing */
	}

	template <typename Iterator>
	Iterator ProxyPropertyPairRange<Iterator>::begin() const
	{
		return _begin;
	}

	template <typename Iterator>
	Iterator ProxyPropertyPairRange<Iterator>::end() const
	{
		return _end;
	}

	template struct ProxyPropertyPairRange<proxy_property_pair_iterator>;
	template struct ProxyPropertyPairRange<proxy_property_pair_iterator_const>;
	template struct ProxyPropertyPairRange<const_proxy_property_pair_iterator_const>;
	template struct ProxyPropertyPairRange<reverse_proxy_property_pair_iterator>;
	template struct ProxyPropertyPairRange<reverse_proxy_property_pair_iterator_const>;
	template struct ProxyPropertyPairRange<const_reverse_proxy_property_pair_iterator_const>;
}