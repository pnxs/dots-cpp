#include <dots/type/PropertyPairIterator.h>

namespace dots::type
{
    template <typename LhsIterator, typename RhsIterator>
    PropertyPairIterator<LhsIterator, RhsIterator>::PropertyPairIterator(inner_lhs_iterator_t innerIteratorLhs, inner_rhs_iterator_t innerIteratorRhs) :
        _innerIteratorLhs(std::move(innerIteratorLhs)),
        _innerIteratorRhs(std::move(innerIteratorRhs))
    {
        /* do nothing */
    }

    template <typename LhsIterator, typename RhsIterator>
    void PropertyPairIterator<LhsIterator, RhsIterator>::swap(PropertyPairIterator& other) noexcept
    {
        std::swap(_innerIteratorLhs, other._innerIteratorLhs);
        std::swap(_innerIteratorRhs, other._innerIteratorRhs);
    }

    template <typename LhsIterator, typename RhsIterator>
    void PropertyPairIterator<LhsIterator, RhsIterator>::swap(PropertyPairIterator&& other)
    {
        _innerIteratorLhs = std::move(other._innerIteratorLhs);
        _innerIteratorRhs = std::move(other._innerIteratorRhs);
    }

    template <typename LhsIterator, typename RhsIterator>
    auto PropertyPairIterator<LhsIterator, RhsIterator>::operator ++ () -> PropertyPairIterator&
    {
        ++_innerIteratorLhs;
        ++_innerIteratorRhs;

        return *this;
    }

    template <typename LhsIterator, typename RhsIterator>
    auto PropertyPairIterator<LhsIterator, RhsIterator>::operator -- () -> PropertyPairIterator&
    {
        --_innerIteratorLhs;
        --_innerIteratorRhs;

        return *this;
    }

    template <typename LhsIterator, typename RhsIterator>
    auto PropertyPairIterator<LhsIterator, RhsIterator>::operator ++ (int) -> PropertyPairIterator
    {
        return PropertyPairIterator{ _innerIteratorLhs++, _innerIteratorRhs++ };
    }

    template <typename LhsIterator, typename RhsIterator>
    auto PropertyPairIterator<LhsIterator, RhsIterator>::operator -- (int) -> PropertyPairIterator
    {
        return PropertyPairIterator{ _innerIteratorLhs--, _innerIteratorRhs-- };
    }

    template <typename LhsIterator, typename RhsIterator>
    auto PropertyPairIterator<LhsIterator, RhsIterator>::operator * () -> reference
    {
        return _proxyPair.emplace(*_innerIteratorLhs, *_innerIteratorRhs);
    }

    template <typename LhsIterator, typename RhsIterator>
    auto PropertyPairIterator<LhsIterator, RhsIterator>::operator * () const -> const_reference
    {
        return *const_cast<PropertyPairIterator&>(*this);
    }

    template <typename LhsIterator, typename RhsIterator>
    auto PropertyPairIterator<LhsIterator, RhsIterator>::operator -> () -> pointer
    {
        return &*(*this);
    }

    template <typename LhsIterator, typename RhsIterator>
    auto PropertyPairIterator<LhsIterator, RhsIterator>::operator -> () const -> const_pointer
    {
        return &*(*this);
    }

    template <typename LhsIterator, typename RhsIterator>
    bool PropertyPairIterator<LhsIterator, RhsIterator>::operator == (const PropertyPairIterator& other) const
    {
        return _innerIteratorLhs == other._innerIteratorLhs && _innerIteratorRhs == other._innerIteratorRhs;
    }

    template <typename LhsIterator, typename RhsIterator>
    bool PropertyPairIterator<LhsIterator, RhsIterator>::operator != (const PropertyPairIterator& other) const
    {
        return !(*this == other);
    }

    template struct PropertyPairIterator<property_iterator, property_iterator>;
    template struct PropertyPairIterator<property_iterator, const_property_iterator>;
    template struct PropertyPairIterator<const_property_iterator, const_property_iterator>;
    template struct PropertyPairIterator<reverse_property_iterator, reverse_property_iterator>;
    template struct PropertyPairIterator<reverse_property_iterator, const_reverse_property_iterator>;
    template struct PropertyPairIterator<const_reverse_property_iterator, const_reverse_property_iterator>;

    template <typename Iterator>
    PropertyPairRange<Iterator>::PropertyPairRange(Iterator begin, Iterator end):
        _begin(std::move(begin)),
        _end(std::move(end))
    {
        /* do nothing */
    }

    template <typename Iterator>
    Iterator PropertyPairRange<Iterator>::begin() const
    {
        return _begin;
    }

    template <typename Iterator>
    Iterator PropertyPairRange<Iterator>::end() const
    {
        return _end;
    }

    template struct PropertyPairRange<property_pair_iterator>;
    template struct PropertyPairRange<property_pair_iterator_const>;
    template struct PropertyPairRange<const_property_pair_iterator_const>;
    template struct PropertyPairRange<reverse_property_pair_iterator>;
    template struct PropertyPairRange<reverse_property_pair_iterator_const>;
    template struct PropertyPairRange<const_reverse_property_pair_iterator_const>;
}