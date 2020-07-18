#pragma once
#include <optional>
#include <dots/type/ProxyProperty.h>

namespace dots::type
{
    template <bool IsReverse, bool IsConst>
    struct PropertyIterator
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
        using difference_type      = std::ptrdiff_t;

        PropertyIterator(area_t& area, const descriptor_container_t& descriptors, descriptor_iterator_t descriptorIt, const PropertySet& properties = PropertySet::All);
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

        bool onBegin() const;
        bool onEnd() const;
        reference emplaceProxy();

        area_t* m_area;
        const descriptor_container_t* m_descriptors;
        descriptor_iterator_t m_descriptorIt;
        PropertySet m_properties;
        std::optional<base_value_type> m_proxy;
    };

    extern template struct PropertyIterator<false, false>;
    extern template struct PropertyIterator<false, true>;
    extern template struct PropertyIterator<true, false>;
    extern template struct PropertyIterator<true, true>;

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

        Iterator m_begin;
        Iterator m_end;
    };

    extern template struct PropertyRange<property_iterator>;
    extern template struct PropertyRange<const_property_iterator>;
    extern template struct PropertyRange<reverse_property_iterator>;
    extern template struct PropertyRange<const_reverse_property_iterator>;

    using property_range               = PropertyRange<property_iterator>;
    using const_property_range         = PropertyRange<const_property_iterator>;
    using reverse_property_range       = PropertyRange<reverse_property_iterator>;
    using const_reverse_property_range = PropertyRange<const_reverse_property_iterator>;
}