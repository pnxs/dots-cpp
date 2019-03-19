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

		PropertyPairIterator(inner_lhs_iterator_t innerIteratorLhs, inner_rhs_iterator_t innerIteratorRhs);
		PropertyPairIterator(const PropertyPairIterator& other) = default;
		PropertyPairIterator(PropertyPairIterator&& other) = default;
		~PropertyPairIterator() = default;

		PropertyPairIterator& operator = (const PropertyPairIterator& rhs) = default;
		PropertyPairIterator& operator = (PropertyPairIterator&& rhs) = default;

		void swap(PropertyPairIterator& other) noexcept;
		void swap(PropertyPairIterator&& other);

		PropertyPairIterator& operator ++ ();
		PropertyPairIterator& operator -- ();

		PropertyPairIterator operator ++ (int);
		PropertyPairIterator operator -- (int);

		reference operator * ();
		const reference operator * () const;

		pointer operator -> ();
		const pointer operator -> () const;

		bool operator == (const PropertyPairIterator& other) const;
		bool operator != (const PropertyPairIterator& other) const;

	private:

		inner_lhs_iterator_t _innerIteratorLhs;
		inner_rhs_iterator_t _innerIteratorRhs;
		std::optional<value_type> _proxyPair;
	};

	extern template	struct PropertyPairIterator<property_iterator, property_iterator>;
	extern template	struct PropertyPairIterator<property_iterator, const_property_iterator>;
	extern template	struct PropertyPairIterator<const_property_iterator, const_property_iterator>;
	extern template	struct PropertyPairIterator<reverse_property_iterator, reverse_property_iterator>;
	extern template	struct PropertyPairIterator<reverse_property_iterator, const_reverse_property_iterator>;
	extern template	struct PropertyPairIterator<const_reverse_property_iterator, const_reverse_property_iterator>;

	using property_pair_iterator                     = PropertyPairIterator<property_iterator, property_iterator>;
	using property_pair_iterator_const               = PropertyPairIterator<property_iterator, const_property_iterator>;
	using const_property_pair_iterator_const         = PropertyPairIterator<const_property_iterator, const_property_iterator>;
	using reverse_property_pair_iterator             = PropertyPairIterator<reverse_property_iterator, reverse_property_iterator>;
	using reverse_property_pair_iterator_const       = PropertyPairIterator<reverse_property_iterator, const_reverse_property_iterator>;
	using const_reverse_property_pair_iterator_const = PropertyPairIterator<const_reverse_property_iterator, const_reverse_property_iterator>;

	template <typename Iterator>
	struct PropertyPairRange
	{
		PropertyPairRange(Iterator begin, Iterator end);
		PropertyPairRange(const PropertyPairRange& other) = default;
		PropertyPairRange(PropertyPairRange&& other) = default;
		~PropertyPairRange() = default;

		PropertyPairRange& operator = (const PropertyPairRange& rhs) = default;
		PropertyPairRange& operator = (PropertyPairRange&& rhs) = default;

		Iterator begin() const;

		Iterator end() const;

	private:

		Iterator _begin;
		Iterator _end;
	};

	extern template	struct PropertyPairRange<property_pair_iterator>;
	extern template	struct PropertyPairRange<property_pair_iterator_const>;
	extern template	struct PropertyPairRange<const_property_pair_iterator_const>;
	extern template	struct PropertyPairRange<reverse_property_pair_iterator>;
	extern template	struct PropertyPairRange<reverse_property_pair_iterator_const>;
	extern template	struct PropertyPairRange<const_reverse_property_pair_iterator_const>;

	using property_pair_range                     = PropertyPairRange<property_pair_iterator>;
	using property_pair_range_const               = PropertyPairRange<property_pair_iterator_const>;
	using const_property_pair_range_const         = PropertyPairRange<const_property_pair_iterator_const>;
	using reverse_property_pair_range             = PropertyPairRange<reverse_property_pair_iterator>;
	using reverse_property_pair_range_const       = PropertyPairRange<reverse_property_pair_iterator_const>;
	using const_reverse_property_pair_range_const = PropertyPairRange<const_reverse_property_pair_iterator_const>;
}