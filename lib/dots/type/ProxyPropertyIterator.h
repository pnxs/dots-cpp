#pragma once
#include <optional>
#include <dots/type/ProxyProperty.h>

namespace dots::type
{
	template <bool IsReverse, bool IsConst>
	struct ProxyPropertyIterator
	{
		// custom iterator traits
		using base_value_type        = ProxyProperty<>;
		using area_t                 = std::conditional_t<IsConst, const PropertyArea, PropertyArea>;
		using descriptor_container_t = std::conditional_t<IsConst, const property_descriptor_container_t, property_descriptor_container_t>;
		using descriptor_iterator_t  = std::conditional_t<IsReverse, typename descriptor_container_t::const_reverse_iterator, typename descriptor_container_t::const_iterator>;

		// STL iterator traits
		using iterator_category    = std::bidirectional_iterator_tag;
		using value_type           = std::conditional_t<IsConst, const base_value_type, base_value_type>;
		using reference            = std::conditional_t<IsConst, const value_type&, value_type&>;
		using pointer              = std::conditional_t<IsConst, const value_type*, value_type*>;

		ProxyPropertyIterator(area_t& area, const descriptor_container_t& descriptors, descriptor_iterator_t descriptorIt, const PropertySet& properties = PropertySet::All);
		ProxyPropertyIterator(const ProxyPropertyIterator& other) = default;
		ProxyPropertyIterator(ProxyPropertyIterator&& other) = default;
		~ProxyPropertyIterator() = default;

		ProxyPropertyIterator& operator = (const ProxyPropertyIterator& rhs) = default;
		ProxyPropertyIterator& operator = (ProxyPropertyIterator&& rhs) = default;

		void swap(ProxyPropertyIterator& other) noexcept;
		void swap(ProxyPropertyIterator&& other);

		ProxyPropertyIterator& operator ++ ();
		ProxyPropertyIterator& operator -- ();

		ProxyPropertyIterator operator ++ (int);
		ProxyPropertyIterator operator -- (int);

		reference operator * ();
		const reference operator * () const;

		pointer operator -> ();
		const pointer operator -> () const;

		bool operator == (const ProxyPropertyIterator& other) const;
		bool operator != (const ProxyPropertyIterator& other) const;

	private:

		bool onBegin() const;
		bool onEnd() const;
		reference emplaceProxy();

		area_t* m_area;
		const descriptor_container_t* m_descriptors;
		descriptor_iterator_t m_descriptorIt;
		PropertySet m_properties;
		std::optional<base_value_type> m_proxy;
	};

	extern template	struct ProxyPropertyIterator<false, false>;
	extern template	struct ProxyPropertyIterator<false, true>;
	extern template	struct ProxyPropertyIterator<true, false>;
	extern template	struct ProxyPropertyIterator<true, true>;

	using proxy_property_iterator               = ProxyPropertyIterator<false, false>;
	using const_proxy_property_iterator         = ProxyPropertyIterator<false, true>;
	using reverse_proxy_property_iterator       = ProxyPropertyIterator<true, false>;
	using const_reverse_proxy_property_iterator = ProxyPropertyIterator<true, true>;

	template <typename Iterator>
	struct ProxyPropertyRange
	{
		ProxyPropertyRange(Iterator begin, Iterator end);
		ProxyPropertyRange(const ProxyPropertyRange& other) = default;
		ProxyPropertyRange(ProxyPropertyRange&& other) = default;
		~ProxyPropertyRange() = default;

		ProxyPropertyRange& operator = (const ProxyPropertyRange& rhs) = default;
		ProxyPropertyRange& operator = (ProxyPropertyRange&& rhs) = default;

		Iterator begin() const;
		Iterator end() const;

	private:

		Iterator m_begin;
		Iterator m_end;
	};

	extern template	struct ProxyPropertyRange<proxy_property_iterator>;
	extern template	struct ProxyPropertyRange<const_proxy_property_iterator>;
	extern template	struct ProxyPropertyRange<reverse_proxy_property_iterator>;
	extern template	struct ProxyPropertyRange<const_reverse_proxy_property_iterator>;

	using proxy_property_range               = ProxyPropertyRange<proxy_property_iterator>;
	using const_proxy_property_range         = ProxyPropertyRange<const_proxy_property_iterator>;
	using reverse_proxy_property_range       = ProxyPropertyRange<reverse_proxy_property_iterator>;
	using const_reverse_proxy_property_range = ProxyPropertyRange<const_reverse_proxy_property_iterator>;
}