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

        PropertyIterator(area_t& area, const descriptor_container_t& descriptors, descriptor_iterator_t descriptorIt, PropertySet properties = PropertySet::All) :
            m_area(&area),
            m_descriptors(&descriptors),
            m_descriptorIt(std::move(descriptorIt)),
            m_properties(properties)
        {
            if (!onEnd())
            {
                if (isPartOf())
                {
                    emplaceProxy();
                }
                else
                {
                    ++*this;
                }
            }
        }
        PropertyIterator(const PropertyIterator& other) = default;
        PropertyIterator(PropertyIterator&& other) = default;
        ~PropertyIterator() = default;

        PropertyIterator& operator = (const PropertyIterator& rhs) = default;
        PropertyIterator& operator = (PropertyIterator&& rhs) = default;

        void swap(PropertyIterator& other) noexcept
        {
            std::swap(m_area, other.m_area);
            std::swap(m_descriptorIt, other.m_descriptorIt);
        }
        void swap(PropertyIterator&& other)
        {
            m_area = other.m_area;
            other.m_area = nullptr;
            m_descriptorIt = std::move(other.m_descriptorIt);
        }

        auto operator ++ () -> PropertyIterator&
        {
            while (!onEnd())
            {
                ++m_descriptorIt;

                if (onEnd())
                {
                    break;
                }
                else if (isPartOf())
                {
                    emplaceProxy();
                    break;
                }
            }

            return *this;
        }

        auto operator -- () -> PropertyIterator&
        {
            while (!onBegin())
            {
                --m_descriptorIt;

                if (isPartOf())
                {
                    emplaceProxy();
                    break;
                }
            }

            return *this;
        }

        auto operator ++ (int) -> PropertyIterator
        {
            PropertyIterator copy = *this;
            ++(*this);

            return copy;
        }

        auto operator -- (int) -> PropertyIterator
        {
            PropertyIterator copy = *this;
            --(*this);

            return copy;
        }

        auto operator * () -> reference
        {
            if (onEnd())
            {
                throw std::logic_error{ "attempt to access past-end property" };
            }

            return *m_proxy;
        }

        auto operator * () const -> const reference
        {
            return *const_cast<PropertyIterator&>(*this);
        }

        auto operator -> () -> pointer
        {
            return &*(*this);
        }

        auto operator -> () const -> const pointer
        {
            return &*(*this);
        }

        bool operator == (const PropertyIterator& other) const
        {
            return m_descriptorIt == other.m_descriptorIt;
        }

        bool operator != (const PropertyIterator& other) const
        {
            return !(*this == other);
        }

    private:

        bool onBegin() const
        {
            if constexpr (IsReverse)
            {
                return m_descriptorIt == m_descriptors->rbegin();
            }
            else
            {
                return m_descriptorIt == m_descriptors->begin();
            }
        }

        bool onEnd() const
        {
            if constexpr (IsReverse)
            {
                return m_descriptorIt == m_descriptors->rend();
            }
            else
            {
                return m_descriptorIt == m_descriptors->end();
            }
        }

        bool isPartOf() const
        {
            return m_descriptorIt->set() <= m_properties;
        }

        reference emplaceProxy()
        {
            if constexpr (IsConst)
            {
                return m_proxy.emplace(const_cast<std::remove_const_t<area_t>&>(*m_area), *m_descriptorIt);
            }
            else
            {
                return m_proxy.emplace(*m_area, *m_descriptorIt);
            }
        }

        area_t* m_area;
        const descriptor_container_t* m_descriptors;
        descriptor_iterator_t m_descriptorIt;
        PropertySet m_properties;
        std::optional<base_value_type> m_proxy;
    };

    using property_iterator               = PropertyIterator<false, false>;
    using const_property_iterator         = PropertyIterator<false, true>;
    using reverse_property_iterator       = PropertyIterator<true, false>;
    using const_reverse_property_iterator = PropertyIterator<true, true>;

    template <typename Iterator>
    struct PropertyRange
    {
        PropertyRange(Iterator begin, Iterator end) :
            m_begin(std::move(begin)),
            m_end(std::move(end))
        {
            /* do nothing */
        }
        PropertyRange(const PropertyRange& other) = default;
        PropertyRange(PropertyRange&& other) = default;
        ~PropertyRange() = default;

        PropertyRange& operator = (const PropertyRange& rhs) = default;
        PropertyRange& operator = (PropertyRange&& rhs) = default;

        Iterator begin() const
        {
            return m_begin;
        }
        Iterator end() const
        {
            return m_end;
        }

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