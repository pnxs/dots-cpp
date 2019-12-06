#pragma once
#include <optional>
#include <utility>
#include <dots/type/NewProxyPropertyIterator.h>

namespace dots::type
{
	template <typename LhsIterator, typename RhsIterator>
	struct ProxyPropertyPairIterator
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
		
		ProxyPropertyPairIterator(inner_lhs_iterator_t innerIteratorLhs, inner_rhs_iterator_t innerIteratorRhs);
		ProxyPropertyPairIterator(const ProxyPropertyPairIterator& other) = default;
		ProxyPropertyPairIterator(ProxyPropertyPairIterator&& other) = default;
		~ProxyPropertyPairIterator() = default;

		ProxyPropertyPairIterator& operator = (const ProxyPropertyPairIterator& rhs) = default;
		ProxyPropertyPairIterator& operator = (ProxyPropertyPairIterator&& rhs) = default;

		void swap(ProxyPropertyPairIterator& other) noexcept;
		void swap(ProxyPropertyPairIterator&& other);

		ProxyPropertyPairIterator& operator ++ ();
		ProxyPropertyPairIterator& operator -- ();

		ProxyPropertyPairIterator operator ++ (int);
		ProxyPropertyPairIterator operator -- (int);

		reference operator * ();
		const_reference operator * () const;

		pointer operator -> ();
		const_pointer operator -> () const;

		bool operator == (const ProxyPropertyPairIterator& other) const;
		bool operator != (const ProxyPropertyPairIterator& other) const;

	private:

		inner_lhs_iterator_t _innerIteratorLhs;
		inner_rhs_iterator_t _innerIteratorRhs;
		std::optional<value_type> _proxyPair;
	};

	extern template	struct ProxyPropertyPairIterator<proxy_property_iterator, proxy_property_iterator>;
	extern template	struct ProxyPropertyPairIterator<proxy_property_iterator, const_proxy_property_iterator>;
	extern template	struct ProxyPropertyPairIterator<const_proxy_property_iterator, const_proxy_property_iterator>;
	extern template	struct ProxyPropertyPairIterator<reverse_proxy_property_iterator, reverse_proxy_property_iterator>;
	extern template	struct ProxyPropertyPairIterator<reverse_proxy_property_iterator, const_reverse_proxy_property_iterator>;
	extern template	struct ProxyPropertyPairIterator<const_reverse_proxy_property_iterator, const_reverse_proxy_property_iterator>;

	using proxy_property_pair_iterator                     = ProxyPropertyPairIterator<proxy_property_iterator, proxy_property_iterator>;
	using proxy_property_pair_iterator_const               = ProxyPropertyPairIterator<proxy_property_iterator, const_proxy_property_iterator>;
	using const_proxy_property_pair_iterator_const         = ProxyPropertyPairIterator<const_proxy_property_iterator, const_proxy_property_iterator>;
	using reverse_proxy_property_pair_iterator             = ProxyPropertyPairIterator<reverse_proxy_property_iterator, reverse_proxy_property_iterator>;
	using reverse_proxy_property_pair_iterator_const       = ProxyPropertyPairIterator<reverse_proxy_property_iterator, const_reverse_proxy_property_iterator>;
	using const_reverse_proxy_property_pair_iterator_const = ProxyPropertyPairIterator<const_reverse_proxy_property_iterator, const_reverse_proxy_property_iterator>;

	template <typename Iterator>
	struct ProxyPropertyPairRange
	{
		ProxyPropertyPairRange(Iterator begin, Iterator end);
		ProxyPropertyPairRange(const ProxyPropertyPairRange& other) = default;
		ProxyPropertyPairRange(ProxyPropertyPairRange&& other) = default;
		~ProxyPropertyPairRange() = default;

		ProxyPropertyPairRange& operator = (const ProxyPropertyPairRange& rhs) = default;
		ProxyPropertyPairRange& operator = (ProxyPropertyPairRange&& rhs) = default;

		Iterator begin() const;

		Iterator end() const;

	private:

		Iterator _begin;
		Iterator _end;
	};

	extern template	struct ProxyPropertyPairRange<proxy_property_pair_iterator>;
	extern template	struct ProxyPropertyPairRange<proxy_property_pair_iterator_const>;
	extern template	struct ProxyPropertyPairRange<const_proxy_property_pair_iterator_const>;
	extern template	struct ProxyPropertyPairRange<reverse_proxy_property_pair_iterator>;
	extern template	struct ProxyPropertyPairRange<reverse_proxy_property_pair_iterator_const>;
	extern template	struct ProxyPropertyPairRange<const_reverse_proxy_property_pair_iterator_const>;

	using proxy_property_pair_range                     = ProxyPropertyPairRange<proxy_property_pair_iterator>;
	using proxy_property_pair_range_const               = ProxyPropertyPairRange<proxy_property_pair_iterator_const>;
	using const_proxy_property_pair_range_const         = ProxyPropertyPairRange<const_proxy_property_pair_iterator_const>;
	using reverse_proxy_property_pair_range             = ProxyPropertyPairRange<reverse_proxy_property_pair_iterator>;
	using reverse_proxy_property_pair_range_const       = ProxyPropertyPairRange<reverse_proxy_property_pair_iterator_const>;
	using const_reverse_proxy_property_pair_range_const = ProxyPropertyPairRange<const_reverse_proxy_property_pair_iterator_const>;
}