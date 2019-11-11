#include <dots/type/NewProxyPropertyPairIterator.h>

namespace dots::type
{
	template <typename LhsIterator, typename RhsIterator>
	NewProxyPropertyPairIterator<LhsIterator, RhsIterator>::NewProxyPropertyPairIterator(inner_lhs_iterator_t innerIteratorLhs, inner_rhs_iterator_t innerIteratorRhs) :
		_innerIteratorLhs(std::move(innerIteratorLhs)),
		_innerIteratorRhs(std::move(innerIteratorRhs))
	{
		/* do nothing */
	}

	template <typename LhsIterator, typename RhsIterator>
	void NewProxyPropertyPairIterator<LhsIterator, RhsIterator>::swap(NewProxyPropertyPairIterator& other) noexcept
	{
		std::swap(_innerIteratorLhs, other._innerIteratorLhs);
		std::swap(_innerIteratorRhs, other._innerIteratorRhs);
	}

	template <typename LhsIterator, typename RhsIterator>
	void NewProxyPropertyPairIterator<LhsIterator, RhsIterator>::swap(NewProxyPropertyPairIterator&& other)
	{
		_innerIteratorLhs = std::move(other._innerIteratorLhs);
		_innerIteratorRhs = std::move(other._innerIteratorRhs);
	}

	template <typename LhsIterator, typename RhsIterator>
	auto NewProxyPropertyPairIterator<LhsIterator, RhsIterator>::operator ++ () -> NewProxyPropertyPairIterator&
	{
		++_innerIteratorLhs;
		++_innerIteratorRhs;

		return *this;
	}

	template <typename LhsIterator, typename RhsIterator>
	auto NewProxyPropertyPairIterator<LhsIterator, RhsIterator>::operator -- () -> NewProxyPropertyPairIterator&
	{
		--_innerIteratorLhs;
		--_innerIteratorRhs;

		return *this;
	}

	template <typename LhsIterator, typename RhsIterator>
	auto NewProxyPropertyPairIterator<LhsIterator, RhsIterator>::operator ++ (int) -> NewProxyPropertyPairIterator
	{
		return NewProxyPropertyPairIterator{ _innerIteratorLhs++, _innerIteratorRhs++ };
	}

	template <typename LhsIterator, typename RhsIterator>
	auto NewProxyPropertyPairIterator<LhsIterator, RhsIterator>::operator -- (int) -> NewProxyPropertyPairIterator
	{
		return NewProxyPropertyPairIterator{ _innerIteratorLhs--, _innerIteratorRhs-- };
	}

	template <typename LhsIterator, typename RhsIterator>
	auto NewProxyPropertyPairIterator<LhsIterator, RhsIterator>::operator * () -> reference
	{
		return _proxyPair.emplace(*_innerIteratorLhs, *_innerIteratorRhs);
	}

	template <typename LhsIterator, typename RhsIterator>
	auto NewProxyPropertyPairIterator<LhsIterator, RhsIterator>::operator * () const -> const_reference
	{
		return *const_cast<NewProxyPropertyPairIterator&>(*this);
	}

	template <typename LhsIterator, typename RhsIterator>
	auto NewProxyPropertyPairIterator<LhsIterator, RhsIterator>::operator -> () -> pointer
	{
		return &*(*this);
	}

	template <typename LhsIterator, typename RhsIterator>
	auto NewProxyPropertyPairIterator<LhsIterator, RhsIterator>::operator -> () const -> const_pointer
	{
		return &*(*this);
	}

	template <typename LhsIterator, typename RhsIterator>
	bool NewProxyPropertyPairIterator<LhsIterator, RhsIterator>::operator == (const NewProxyPropertyPairIterator& other) const
	{
		return _innerIteratorLhs == other._innerIteratorLhs && _innerIteratorRhs == other._innerIteratorRhs;
	}

	template <typename LhsIterator, typename RhsIterator>
	bool NewProxyPropertyPairIterator<LhsIterator, RhsIterator>::operator != (const NewProxyPropertyPairIterator& other) const
	{
		return !(*this == other);
	}

	template struct NewProxyPropertyPairIterator<new_proxy_property_iterator, new_proxy_property_iterator>;
	template struct NewProxyPropertyPairIterator<new_proxy_property_iterator, new_const_proxy_property_iterator>;
	template struct NewProxyPropertyPairIterator<new_const_proxy_property_iterator, new_const_proxy_property_iterator>;
	template struct NewProxyPropertyPairIterator<new_reverse_proxy_property_iterator, new_reverse_proxy_property_iterator>;
	template struct NewProxyPropertyPairIterator<new_reverse_proxy_property_iterator, new_const_reverse_proxy_property_iterator>;
	template struct NewProxyPropertyPairIterator<new_const_reverse_proxy_property_iterator, new_const_reverse_proxy_property_iterator>;

	template <typename Iterator>
	NewProxyPropertyPairRange<Iterator>::NewProxyPropertyPairRange(Iterator begin, Iterator end):
		_begin(std::move(begin)),
		_end(std::move(end))
	{
		/* do nothing */
	}

	template <typename Iterator>
	Iterator NewProxyPropertyPairRange<Iterator>::begin() const
	{
		return _begin;
	}

	template <typename Iterator>
	Iterator NewProxyPropertyPairRange<Iterator>::end() const
	{
		return _end;
	}

	template struct NewProxyPropertyPairRange<new_proxy_property_pair_iterator>;
	template struct NewProxyPropertyPairRange<new_proxy_property_pair_iterator_const>;
	template struct NewProxyPropertyPairRange<new_const_proxy_property_pair_iterator_const>;
	template struct NewProxyPropertyPairRange<new_reverse_proxy_property_pair_iterator>;
	template struct NewProxyPropertyPairRange<new_reverse_proxy_property_pair_iterator_const>;
	template struct NewProxyPropertyPairRange<new_const_reverse_proxy_property_pair_iterator_const>;
}