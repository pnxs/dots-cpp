#pragma once
#include <optional>
#include <utility>
#include <dots/type/NewProxyPropertyIterator.h>

namespace dots::type
{
	template <typename LhsIterator, typename RhsIterator>
	struct NewProxyPropertyPairIterator
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
		
		NewProxyPropertyPairIterator(inner_lhs_iterator_t innerIteratorLhs, inner_rhs_iterator_t innerIteratorRhs);
		NewProxyPropertyPairIterator(const NewProxyPropertyPairIterator& other) = default;
		NewProxyPropertyPairIterator(NewProxyPropertyPairIterator&& other) = default;
		~NewProxyPropertyPairIterator() = default;

		NewProxyPropertyPairIterator& operator = (const NewProxyPropertyPairIterator& rhs) = default;
		NewProxyPropertyPairIterator& operator = (NewProxyPropertyPairIterator&& rhs) = default;

		void swap(NewProxyPropertyPairIterator& other) noexcept;
		void swap(NewProxyPropertyPairIterator&& other);

		NewProxyPropertyPairIterator& operator ++ ();
		NewProxyPropertyPairIterator& operator -- ();

		NewProxyPropertyPairIterator operator ++ (int);
		NewProxyPropertyPairIterator operator -- (int);

		reference operator * ();
		const_reference operator * () const;

		pointer operator -> ();
		const_pointer operator -> () const;

		bool operator == (const NewProxyPropertyPairIterator& other) const;
		bool operator != (const NewProxyPropertyPairIterator& other) const;

	private:

		inner_lhs_iterator_t _innerIteratorLhs;
		inner_rhs_iterator_t _innerIteratorRhs;
		std::optional<value_type> _proxyPair;
	};

	extern template	struct NewProxyPropertyPairIterator<new_proxy_property_iterator, new_proxy_property_iterator>;
	extern template	struct NewProxyPropertyPairIterator<new_proxy_property_iterator, new_const_proxy_property_iterator>;
	extern template	struct NewProxyPropertyPairIterator<new_const_proxy_property_iterator, new_const_proxy_property_iterator>;
	extern template	struct NewProxyPropertyPairIterator<new_reverse_proxy_property_iterator, new_reverse_proxy_property_iterator>;
	extern template	struct NewProxyPropertyPairIterator<new_reverse_proxy_property_iterator, new_const_reverse_proxy_property_iterator>;
	extern template	struct NewProxyPropertyPairIterator<new_const_reverse_proxy_property_iterator, new_const_reverse_proxy_property_iterator>;

	using new_proxy_property_pair_iterator                     = NewProxyPropertyPairIterator<new_proxy_property_iterator, new_proxy_property_iterator>;
	using new_proxy_property_pair_iterator_const               = NewProxyPropertyPairIterator<new_proxy_property_iterator, new_const_proxy_property_iterator>;
	using new_const_proxy_property_pair_iterator_const         = NewProxyPropertyPairIterator<new_const_proxy_property_iterator, new_const_proxy_property_iterator>;
	using new_reverse_proxy_property_pair_iterator             = NewProxyPropertyPairIterator<new_reverse_proxy_property_iterator, new_reverse_proxy_property_iterator>;
	using new_reverse_proxy_property_pair_iterator_const       = NewProxyPropertyPairIterator<new_reverse_proxy_property_iterator, new_const_reverse_proxy_property_iterator>;
	using new_const_reverse_proxy_property_pair_iterator_const = NewProxyPropertyPairIterator<new_const_reverse_proxy_property_iterator, new_const_reverse_proxy_property_iterator>;

	template <typename Iterator>
	struct NewProxyPropertyPairRange
	{
		NewProxyPropertyPairRange(Iterator begin, Iterator end);
		NewProxyPropertyPairRange(const NewProxyPropertyPairRange& other) = default;
		NewProxyPropertyPairRange(NewProxyPropertyPairRange&& other) = default;
		~NewProxyPropertyPairRange() = default;

		NewProxyPropertyPairRange& operator = (const NewProxyPropertyPairRange& rhs) = default;
		NewProxyPropertyPairRange& operator = (NewProxyPropertyPairRange&& rhs) = default;

		Iterator begin() const;

		Iterator end() const;

	private:

		Iterator _begin;
		Iterator _end;
	};

	extern template	struct NewProxyPropertyPairRange<new_proxy_property_pair_iterator>;
	extern template	struct NewProxyPropertyPairRange<new_proxy_property_pair_iterator_const>;
	extern template	struct NewProxyPropertyPairRange<new_const_proxy_property_pair_iterator_const>;
	extern template	struct NewProxyPropertyPairRange<new_reverse_proxy_property_pair_iterator>;
	extern template	struct NewProxyPropertyPairRange<new_reverse_proxy_property_pair_iterator_const>;
	extern template	struct NewProxyPropertyPairRange<new_const_reverse_proxy_property_pair_iterator_const>;

	using new_proxy_property_pair_range                     = NewProxyPropertyPairRange<new_proxy_property_pair_iterator>;
	using new_proxy_property_pair_range_const               = NewProxyPropertyPairRange<new_proxy_property_pair_iterator_const>;
	using new_const_proxy_property_pair_range_const         = NewProxyPropertyPairRange<new_const_proxy_property_pair_iterator_const>;
	using new_reverse_proxy_property_pair_range             = NewProxyPropertyPairRange<new_reverse_proxy_property_pair_iterator>;
	using new_reverse_proxy_property_pair_range_const       = NewProxyPropertyPairRange<new_reverse_proxy_property_pair_iterator_const>;
	using new_const_reverse_proxy_property_pair_range_const = NewProxyPropertyPairRange<new_const_reverse_proxy_property_pair_iterator_const>;
}