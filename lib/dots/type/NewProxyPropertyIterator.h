#pragma once
#include <optional>
#include <dots/type/NewProxyProperty.h>

namespace dots::type
{
	template <bool IsReverse, bool IsConst>
	struct NewProxyPropertyIterator
	{
		// custom iterator traits
		using base_value_type        = NewProxyProperty<>;
		using area_t                 = std::conditional_t<IsConst, const NewPropertyArea, NewPropertyArea>;
		using descriptor_container_t = std::conditional_t<IsConst, const new_property_descriptor_container_t, new_property_descriptor_container_t>;
		using descriptor_iterator_t  = std::conditional_t<IsReverse, typename descriptor_container_t::const_reverse_iterator, typename descriptor_container_t::const_iterator>;

		// STL iterator traits
		using iterator_category    = std::bidirectional_iterator_tag;
		using value_type           = std::conditional_t<IsConst, const base_value_type, base_value_type>;
		using reference            = std::conditional_t<IsConst, const value_type&, value_type&>;
		using pointer              = std::conditional_t<IsConst, const value_type*, value_type*>;

		NewProxyPropertyIterator(area_t& area, const descriptor_container_t& descriptors, descriptor_iterator_t descriptorIt, const NewPropertySet& properties = NewPropertySet::All);
		NewProxyPropertyIterator(const NewProxyPropertyIterator& other) = default;
		NewProxyPropertyIterator(NewProxyPropertyIterator&& other) = default;
		~NewProxyPropertyIterator() = default;

		NewProxyPropertyIterator& operator = (const NewProxyPropertyIterator& rhs) = default;
		NewProxyPropertyIterator& operator = (NewProxyPropertyIterator&& rhs) = default;

		void swap(NewProxyPropertyIterator& other) noexcept;
		void swap(NewProxyPropertyIterator&& other);

		NewProxyPropertyIterator& operator ++ ();
		NewProxyPropertyIterator& operator -- ();

		NewProxyPropertyIterator operator ++ (int);
		NewProxyPropertyIterator operator -- (int);

		reference operator * ();
		const reference operator * () const;

		pointer operator -> ();
		const pointer operator -> () const;

		bool operator == (const NewProxyPropertyIterator& other) const;
		bool operator != (const NewProxyPropertyIterator& other) const;

	private:

		bool onBegin() const;
		bool onEnd() const;
		reference emplaceProxy();

		area_t* m_area;
		const descriptor_container_t* m_descriptors;
		descriptor_iterator_t m_descriptorIt;
		NewPropertySet m_properties;
		std::optional<base_value_type> m_proxy;
	};

	extern template	struct NewProxyPropertyIterator<false, false>;
	extern template	struct NewProxyPropertyIterator<false, true>;
	extern template	struct NewProxyPropertyIterator<true, false>;
	extern template	struct NewProxyPropertyIterator<true, true>;

	using new_proxy_property_iterator               = NewProxyPropertyIterator<false, false>;
	using new_const_proxy_property_iterator         = NewProxyPropertyIterator<false, true>;
	using new_reverse_proxy_property_iterator       = NewProxyPropertyIterator<true, false>;
	using new_const_reverse_proxy_property_iterator = NewProxyPropertyIterator<true, true>;

	template <typename Iterator>
	struct NewProxyPropertyRange
	{
		NewProxyPropertyRange(Iterator begin, Iterator end);
		NewProxyPropertyRange(const NewProxyPropertyRange& other) = default;
		NewProxyPropertyRange(NewProxyPropertyRange&& other) = default;
		~NewProxyPropertyRange() = default;

		NewProxyPropertyRange& operator = (const NewProxyPropertyRange& rhs) = default;
		NewProxyPropertyRange& operator = (NewProxyPropertyRange&& rhs) = default;

		Iterator begin() const;
		Iterator end() const;

	private:

		Iterator m_begin;
		Iterator m_end;
	};

	extern template	struct NewProxyPropertyRange<new_proxy_property_iterator>;
	extern template	struct NewProxyPropertyRange<new_const_proxy_property_iterator>;
	extern template	struct NewProxyPropertyRange<new_reverse_proxy_property_iterator>;
	extern template	struct NewProxyPropertyRange<new_const_reverse_proxy_property_iterator>;

	using new_proxy_property_range               = NewProxyPropertyRange<new_proxy_property_iterator>;
	using new_const_proxy_property_range         = NewProxyPropertyRange<new_const_proxy_property_iterator>;
	using new_reverse_proxy_property_range       = NewProxyPropertyRange<new_reverse_proxy_property_iterator>;
	using new_const_reverse_proxy_property_range = NewProxyPropertyRange<new_const_reverse_proxy_property_iterator>;
}