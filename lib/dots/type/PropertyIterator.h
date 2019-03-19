#pragma once
#include <optional>
#include "PropertyProxy.h"

namespace dots::type
{
	template <bool IsReverse, bool IsConst>
	struct PropertyIterator
	{
		// custom iterator traits
		using base_value_type   = PropertyProxy<void>;
		using struct_t          = std::conditional_t<IsConst, const Struct, Struct>;
		using inner_container_t = std::remove_reference_t<decltype(std::declval<struct_t>()._descriptor().properties())>;
		using inner_iterator_t  = std::conditional_t<IsReverse, typename inner_container_t::const_reverse_iterator, typename inner_container_t::const_iterator>;

		// STL iterator traits
		using iterator_category = typename  inner_iterator_t::iterator_category;
		using value_type        = std::conditional_t<IsConst, const base_value_type, base_value_type>;
		using reference         = std::conditional_t<IsConst, const value_type&, value_type&>;
		using pointer           = std::conditional_t<IsConst, const value_type*, value_type*>;

		PropertyIterator(struct_t& instance, inner_iterator_t innerIterator, const property_set& set = PROPERTY_SET_ALL);
		PropertyIterator(const PropertyIterator& other) = default;
		PropertyIterator(PropertyIterator&& other) = default;
		~PropertyIterator() = default;

		PropertyIterator& operator = (const PropertyIterator& rhs) = default;
		PropertyIterator& operator = (PropertyIterator&& rhs) = default;

		void swap(PropertyIterator& other) noexcept;
		void swap(PropertyIterator&& other);

		PropertyIterator& operator ++ ();
		PropertyIterator& operator -- ();

		PropertyIterator operator ++ (int);
		PropertyIterator operator -- (int);

		reference operator * ();
		const reference operator * () const;

		pointer operator -> ();
		const pointer operator -> () const;

		bool operator == (const PropertyIterator& other) const;
		bool operator != (const PropertyIterator& other) const;

	private:

		bool hasReachedEnd() const;

		reference emplaceProxy();

		struct_t* _instance;
		inner_iterator_t _innerIterator;
		property_set _set;
		std::optional<base_value_type> _proxy;
	};

	extern template	struct PropertyIterator<false, false>;
	extern template	struct PropertyIterator<false, true>;
	extern template	struct PropertyIterator<true, false>;
	extern template	struct PropertyIterator<true, true>;

	using property_iterator               = PropertyIterator<false, false>;
	using const_property_iterator         = PropertyIterator<false, true>;
	using reverse_property_iterator       = PropertyIterator<true, false>;
	using const_reverse_property_iterator = PropertyIterator<true, true>;

	template <typename Iterator>
	struct PropertyRange
	{
		PropertyRange(Iterator begin, Iterator end);
		PropertyRange(const PropertyRange& other) = default;
		PropertyRange(PropertyRange&& other) = default;
		~PropertyRange() = default;

		PropertyRange& operator = (const PropertyRange& rhs) = default;
		PropertyRange& operator = (PropertyRange&& rhs) = default;

		Iterator begin() const;
		Iterator end() const;

	private:

		Iterator _begin;
		Iterator _end;
	};

	extern template	struct PropertyRange<property_iterator>;
	extern template	struct PropertyRange<const_property_iterator>;
	extern template	struct PropertyRange<reverse_property_iterator>;
	extern template	struct PropertyRange<const_reverse_property_iterator>;

	using property_range               = PropertyRange<property_iterator>;
	using const_property_range         = PropertyRange<const_property_iterator>;
	using reverse_property_range       = PropertyRange<reverse_property_iterator>;
	using const_reverse_property_range = PropertyRange<const_reverse_property_iterator>;
}