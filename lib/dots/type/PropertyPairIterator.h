#pragma once
#include <optional>
#include <utility>
#include "PropertyIterator.h"

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
		using reference         = std::pair<inner_lhs_value_t, inner_rhs_value_t>&;
		using pointer           = std::pair<inner_lhs_value_t, inner_rhs_value_t>*;

		PropertyPairIterator(inner_lhs_iterator_t innerIteratorLhs, inner_rhs_iterator_t innerIteratorRhs) :
			_innerIteratorLhs(std::move(innerIteratorLhs)),
			_innerIteratorRhs(std::move(innerIteratorRhs))
		{
			/* do nothing */
		}
		PropertyPairIterator(const PropertyPairIterator& other) = default;
		PropertyPairIterator(PropertyPairIterator&& other) = default;
		~PropertyPairIterator() = default;

		PropertyPairIterator& operator = (const PropertyPairIterator& rhs) = default;
		PropertyPairIterator& operator = (PropertyPairIterator&& rhs) = default;

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

		PropertyPairIterator& operator ++ ()
		{
			++_innerIteratorLhs;
			++_innerIteratorRhs;

			return *this;
		}

		PropertyPairIterator& operator -- ()
		{
			--_innerIteratorLhs;
			--_innerIteratorRhs;

			return *this;
		}

		PropertyPairIterator operator ++ (int)
		{
			return PropertyPairIterator{ _innerIteratorLhs++, _innerIteratorRhs++ };
		}

		PropertyPairIterator operator -- (int)
		{
			return PropertyPairIterator{ _innerIteratorLhs--, _innerIteratorRhs-- };
		}

		reference operator * ()
		{
			return _proxyPair.emplace(*_innerIteratorLhs, *_innerIteratorRhs);
		}

		const reference operator * () const
		{
			return *const_cast<PropertyPairIterator&>(*this);
		}

		pointer operator -> ()
		{
			return &*(*this);
		}

		const pointer operator -> () const
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

	struct property_pair_iterator : PropertyPairIterator<property_iterator, property_iterator>
	{
		using PropertyPairIterator::PropertyPairIterator;
	};

	struct property_pair_iterator_const : PropertyPairIterator<property_iterator, const_property_iterator>
	{
		using PropertyPairIterator::PropertyPairIterator;
	};

	struct const_property_pair_iterator_const : PropertyPairIterator<const_property_iterator, const_property_iterator>
	{
		using PropertyPairIterator::PropertyPairIterator;
	};

	struct reverse_property_pair_iterator : PropertyPairIterator<reverse_property_iterator, reverse_property_iterator>
	{
		using PropertyPairIterator::PropertyPairIterator;
	};

	struct reverse_property_pair_iterator_const : PropertyPairIterator<reverse_property_iterator, const_reverse_property_iterator>
	{
		using PropertyPairIterator::PropertyPairIterator;
	};

	struct const_reverse_property_pair_iterator_const : PropertyPairIterator<const_reverse_property_iterator, const_reverse_property_iterator>
	{
		using PropertyPairIterator::PropertyPairIterator;
	};

	template <typename Iterator>
	struct PropertyPairRange
	{
		PropertyPairRange(Iterator begin, Iterator end) :
			_begin(std::move(begin)),
			_end(std::move(end))
		{
			/* do nothing */
		}
		PropertyPairRange(const PropertyPairRange& other) = default;
		PropertyPairRange(PropertyPairRange&& other) = default;
		~PropertyPairRange() = default;

		PropertyPairRange& operator = (const PropertyPairRange& rhs) = default;
		PropertyPairRange& operator = (PropertyPairRange&& rhs) = default;

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

	struct property_pair_range : PropertyPairRange<property_pair_iterator>
	{
		using PropertyPairRange::PropertyPairRange;
	};

	struct property_pair_range_const : PropertyPairRange<property_pair_iterator_const>
	{
		using PropertyPairRange::PropertyPairRange;
	};

	struct const_property_pair_range_const : PropertyPairRange<const_property_pair_iterator_const>
	{
		using PropertyPairRange::PropertyPairRange;
	};

	struct reverse_property_pair_range : PropertyPairRange<reverse_property_pair_iterator>
	{
		using PropertyPairRange::PropertyPairRange;
	};

	struct reverse_property_pair_range_const : PropertyPairRange<reverse_property_pair_iterator_const>
	{
		using PropertyPairRange::PropertyPairRange;
	};

	struct const_reverse_property_pair_range_const : PropertyPairRange<const_reverse_property_pair_iterator_const>
	{
		using PropertyPairRange::PropertyPairRange;
	};
}