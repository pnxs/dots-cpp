#pragma once
#include <optional>
#include <utility>
#include <dots/type/PropertyIterator.h>

namespace dots::type
{
    template <typename LhsIterator, typename RhsIterator>
    struct PropertyPairIterator
    {
        // custom iterator traits
        using inner_lhs_iterator_t          = LhsIterator;
        using inner_rhs_iterator_t          = RhsIterator;
        using inner_lhs_value_t             = typename inner_lhs_iterator_t::value_type;
        using inner_rhs_value_t             = typename inner_rhs_iterator_t::value_type;
        using inner_lhs_iterator_category_t = typename inner_lhs_iterator_t::iterator_category;
        using inner_rhs_iterator_category_t = typename inner_rhs_iterator_t::iterator_category;
        static_assert(std::is_same_v<inner_lhs_iterator_category_t, inner_rhs_iterator_category_t>, "iterator types must have same category");

        // STL iterator traits
        using iterator_category = typename inner_lhs_iterator_t::iterator_category;
        using value_type        = std::pair<inner_lhs_value_t, inner_rhs_value_t>;
        using reference         = value_type&;
        using const_reference   = const value_type&;
        using pointer           = value_type*;
        using const_pointer     = const value_type*;
        using difference_type   = typename inner_lhs_iterator_t::difference_type;

        PropertyPairIterator(inner_lhs_iterator_t innerIteratorLhs, inner_rhs_iterator_t innerIteratorRhs) :
            _innerIteratorLhs(std::move(innerIteratorLhs)),
            _innerIteratorRhs(std::move(innerIteratorRhs))
        {
            /* do nothing */
        }

        void swap(PropertyPairIterator& other) noexcept
        {
            std::swap(_innerIteratorLhs, other._innerIteratorLhs);
            std::swap(_innerIteratorRhs, other._innerIteratorRhs);
        }

        void swap(PropertyPairIterator&& other)
        {
            _innerIteratorLhs = std::move(other._innerIteratorLhs);
            _innerIteratorRhs = std::move(other._innerIteratorRhs);
        }

        auto operator ++ () -> PropertyPairIterator&
        {
            ++_innerIteratorLhs;
            ++_innerIteratorRhs;

            return *this;
        }

        auto operator -- () -> PropertyPairIterator&
        {
            --_innerIteratorLhs;
            --_innerIteratorRhs;

            return *this;
        }

        auto operator ++ (int) -> PropertyPairIterator
        {
            return PropertyPairIterator{ _innerIteratorLhs++, _innerIteratorRhs++ };
        }

        auto operator -- (int) -> PropertyPairIterator
        {
            return PropertyPairIterator{ _innerIteratorLhs--, _innerIteratorRhs-- };
        }

        auto operator * () -> reference
        {
            return _proxyPair.emplace(*_innerIteratorLhs, *_innerIteratorRhs);
        }

        auto operator * () const -> const_reference
        {
            return *const_cast<PropertyPairIterator&>(*this);
        }

        auto operator -> () -> pointer
        {
            return &*(*this);
        }

        auto operator -> () const -> const_pointer
        {
            return &*(*this);
        }

        bool operator == (const PropertyPairIterator& other) const
        {
            return _innerIteratorLhs == other._innerIteratorLhs && _innerIteratorRhs == other._innerIteratorRhs;
        }

        bool operator != (const PropertyPairIterator& other) const
        {
            return !(*this == other);
        }

    private:

        inner_lhs_iterator_t _innerIteratorLhs;
        inner_rhs_iterator_t _innerIteratorRhs;
        std::optional<value_type> _proxyPair;
    };

    using property_pair_iterator                     = PropertyPairIterator<property_iterator, property_iterator>;
    using property_pair_iterator_const               = PropertyPairIterator<property_iterator, const_property_iterator>;
    using const_property_pair_iterator_const         = PropertyPairIterator<const_property_iterator, const_property_iterator>;
    using reverse_property_pair_iterator             = PropertyPairIterator<reverse_property_iterator, reverse_property_iterator>;
    using reverse_property_pair_iterator_const       = PropertyPairIterator<reverse_property_iterator, const_reverse_property_iterator>;
    using const_reverse_property_pair_iterator_const = PropertyPairIterator<const_reverse_property_iterator, const_reverse_property_iterator>;

    template <typename Iterator>
    struct PropertyPairRange
    {
        PropertyPairRange(Iterator begin, Iterator end):
            _begin(std::move(begin)),
            _end(std::move(end))
        {
            /* do nothing */
        }

        Iterator begin() const
        {
            return _begin;
        }

        Iterator end() const
        {
            return _end;
        }

    private:

        Iterator _begin;
        Iterator _end;
    };

    using property_pair_range                     = PropertyPairRange<property_pair_iterator>;
    using property_pair_range_const               = PropertyPairRange<property_pair_iterator_const>;
    using const_property_pair_range_const         = PropertyPairRange<const_property_pair_iterator_const>;
    using reverse_property_pair_range             = PropertyPairRange<reverse_property_pair_iterator>;
    using reverse_property_pair_range_const       = PropertyPairRange<reverse_property_pair_iterator_const>;
    using const_reverse_property_pair_range_const = PropertyPairRange<const_reverse_property_pair_iterator_const>;
}
