#pragma once
#include <deque>
#include <dots/type/Descriptor.h>
#include <dots/type/StaticDescriptor.h>
#include <dots/type/Property.h>
#include <dots/type/PropertyPath.h>

namespace dots::types
{
    struct StructDescriptorData;
}

namespace dots::type
{
    struct Struct;

    template <typename T = Typeless, bool = details::use_static_descriptor_operations_v<T>, typename = void>
    struct StructDescriptor;

    template <>
    struct StructDescriptor<Typeless> : Descriptor<Typeless>
    {
        static const uint8_t Uncached      = 0b0000'0000;
        static const uint8_t Cached        = 0b0000'0001;
        static const uint8_t Internal      = 0b0000'0010;
        static const uint8_t Persistent    = 0b0000'0100;
        static const uint8_t Cleanup       = 0b0000'1000;
        static const uint8_t Local         = 0b0001'0000;
        static const uint8_t SubstructOnly = 0b0010'0000;

        StructDescriptor(std::string name, uint8_t flags, const property_descriptor_container_t& propertyDescriptors, size_t areaOffset, size_t size, size_t alignment);
        StructDescriptor(const StructDescriptor& other) = default;
        StructDescriptor(StructDescriptor&& other) = default;
        ~StructDescriptor() = default;

        StructDescriptor& operator = (const StructDescriptor& rhs) = default;
        StructDescriptor& operator = (StructDescriptor&& rhs) = default;

        Typeless& construct(Typeless& value) const override;
        Struct& construct(Struct& instance) const;
        Typeless& construct(Typeless& value, const Typeless& other) const override;
        Struct& construct(Struct& instance, const Struct& other) const;
        Typeless& construct(Typeless& value, Typeless&& other) const override;
        Struct& construct(Struct& instance, Struct&& other) const;

        Typeless& constructInPlace(Typeless& value) const override;
        Struct& constructInPlace(Struct& instance) const;
        Typeless& constructInPlace(Typeless& value, const Typeless& other) const override;
        Struct& constructInPlace(Struct& instance, const Struct& other) const;
        Typeless& constructInPlace(Typeless& value, Typeless&& other) const override;
        Struct& constructInPlace(Struct& instance, Struct&& other) const;

        void destruct(Typeless& value) const override;
        Struct& destruct(Struct& instance) const;

        Typeless& assign(Typeless& lhs, const Typeless& rhs) const override;
        Typeless& assign(Typeless& lhs, Typeless&& rhs) const override;
        void swap(Typeless& value, Typeless& other) const override;

        bool equal(const Typeless& lhs, const Typeless& rhs) const override;
        bool less(const Typeless& lhs, const Typeless& rhs) const override;
        bool lessEqual(const Typeless& lhs, const Typeless& rhs) const override;
        bool greater(const Typeless& lhs, const Typeless& rhs) const override;
        bool greaterEqual(const Typeless& lhs, const Typeless& rhs) const override;

        size_t areaOffset() const;
        size_t numSubStructs() const;

        bool usesDynamicMemory() const override;
        size_t dynamicMemoryUsage(const Typeless& instance) const override;
        size_t dynamicMemoryUsage(const Struct& instance) const;

        virtual Struct& assign(Struct& instance, const Struct& other, const PropertySet& includedProperties) const;
        virtual Struct& assign(Struct& instance, Struct&& other, const PropertySet& includedProperties) const;
        virtual Struct& copy(Struct& instance, const Struct& other, const PropertySet& includedProperties) const;
        virtual Struct& merge(Struct& instance, const Struct& other, const PropertySet& includedProperties) const;
        virtual void swap(Struct& instance, Struct& other, const PropertySet& includedProperties) const;
        virtual void clear(Struct& instance, const PropertySet& includedProperties) const;

        virtual bool equal(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const;
        virtual bool same(const Struct& lhs, const Struct& rhs) const;

        virtual bool less(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const;
        virtual bool lessEqual(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const;
        virtual bool greater(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const;
        virtual bool greaterEqual(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const;

        virtual PropertySet diffProperties(const Struct& instance, const Struct& other, const PropertySet& includedProperties) const;

        virtual const PropertyArea& propertyArea(const Struct& instance) const = 0;
        virtual PropertyArea& propertyArea(Struct& instance) const = 0;

        uint8_t flags() const;
        bool cached() const;
        bool cleanup() const;
        bool local() const;
        bool persistent() const;
        bool internal() const;
        bool substructOnly() const;

        const property_descriptor_container_t& propertyDescriptors() const;
        partial_property_descriptor_container_t propertyDescriptors(const PropertySet& properties) const;

        const std::vector<PropertyPath>& propertyPaths() const;

        const PropertySet& properties() const;
        const PropertySet& keyProperties() const;

        [[deprecated("only available for backwards compatibility")]]
        const PropertySet& keys() const;

        [[deprecated("only available for backwards compatibility")]]
        const PropertySet& validProperties(const void* instance) const;

        [[deprecated("only available for backwards compatibility")]]
        PropertySet& validProperties(void* instance) const;

        #ifndef DOTS_NO_GLOBAL_TRANSCEIVER

        [[deprecated("only available for backwards compatibility")]]
        const types::StructDescriptorData& descriptorData() const;

        [[deprecated("only available for backwards compatibility")]]
        static const StructDescriptor<>* createFromStructDescriptorData(const types::StructDescriptorData& sd);

        #endif

    private:

        uint8_t m_flags;
        property_descriptor_container_t m_propertyDescriptors;
        size_t m_areaOffset;
        PropertySet m_properties;
        PropertySet m_keyProperties;
        size_t m_numSubStructs;
        PropertySet m_dynamicMemoryProperties;
        mutable property_descriptor_container_t m_flatPropertyDescriptors;
        mutable std::deque<PropertyDescriptor> m_subAreaPropertyDescriptors;
        mutable std::vector<PropertyPath> m_propertyPaths;
        mutable const types::StructDescriptorData* m_descriptorData = nullptr;
    };

    template <typename T>
    struct StructDescriptor<T, false> : StaticDescriptor<T, StructDescriptor<Typeless>>
    {
        static_assert(std::is_base_of_v<Struct, T>);

        StructDescriptor(std::string name, uint8_t flags, const property_descriptor_container_t& propertyDescriptor) :
            StaticDescriptor<T, StructDescriptor<Typeless>>(std::move(name), flags, propertyDescriptor, sizeof(const StructDescriptor<>*), sizeof(T), alignof(T))
        {
            /* do nothing */
        }
        StructDescriptor(std::string name, uint8_t flags, const property_descriptor_container_t& propertyDescriptor, size_t areaOffset, size_t size, size_t alignment) :
            StaticDescriptor<T, StructDescriptor<Typeless>>(std::move(name), flags, propertyDescriptor, areaOffset, size, alignment)
        {
            /* do nothing */
        }
        StructDescriptor(const StructDescriptor& other) = default;
        StructDescriptor(StructDescriptor&& other) = default;
        ~StructDescriptor() = default;

        StructDescriptor& operator = (const StructDescriptor& rhs) = default;
        StructDescriptor& operator = (StructDescriptor&& rhs) = default;

        using StaticDescriptor<T, StructDescriptor<Typeless>>::assign;
        using StaticDescriptor<T, StructDescriptor<Typeless>>::swap;
        using StaticDescriptor<T, StructDescriptor<Typeless>>::equal;
        using StaticDescriptor<T, StructDescriptor<Typeless>>::less;
        using StaticDescriptor<T, StructDescriptor<Typeless>>::lessEqual;
        using StaticDescriptor<T, StructDescriptor<Typeless>>::greater;
        using StaticDescriptor<T, StructDescriptor<Typeless>>::greaterEqual;

        const PropertyArea& propertyArea(const Struct& instance) const override
        {
            return StructDescriptor::propertyArea(static_cast<const T&>(instance));
        }

        PropertyArea& propertyArea(Struct& instance) const override
        {
            return StructDescriptor::propertyArea(static_cast<T&>(instance));
        }

        static T& assign(T& instance, const T& other, const PropertySet& includedProperties)
        {
            return instance._assign(other, includedProperties);
        }

        static T& assign(T& instance, T&& other, const PropertySet& includedProperties)
        {
            return instance._assign(std::move(other), includedProperties);
        }

        static T& copy(T& instance, const T& other, const PropertySet& includedProperties)
        {
            return instance._copy(other, includedProperties);
        }

        static T& merge(T& instance, const T& other, const PropertySet& includedProperties)
        {
            return instance._merge(other, includedProperties);
        }

        static void swap(T& instance, T& other, const PropertySet& includedProperties)
        {
            instance._swap(other, includedProperties);
        }

        static void clear(T& instance, const PropertySet& includedProperties)
        {
            instance._clear(includedProperties);
        }

        static bool equal(const T& lhs, const T& rhs, const PropertySet& includedProperties)
        {
            return lhs._equal(rhs, includedProperties);
        }

        static bool same(const T& lhs, const T& rhs)
        {
            return lhs._same(rhs);
        }

        static bool less(const T& lhs, const T& rhs, const PropertySet& includedProperties)
        {
            return lhs._less(rhs, includedProperties);
        }

        static bool lessEqual(const T& lhs, const T& rhs, const PropertySet& includedProperties)
        {
            return lhs._lessEqual(rhs, includedProperties);
        }

        static bool greater(const T& lhs, const T& rhs, const PropertySet& includedProperties)
        {
            return lhs._greater(rhs, includedProperties);
        }

        static bool greaterEqual(const T& lhs, const T& rhs, const PropertySet& includedProperties)
        {
            return lhs._greaterEqual(rhs, includedProperties);
        }

        static PropertySet diffProperties(const T& instance, const T& other, const PropertySet& includedProperties)
        {
            return instance._diffProperties(other, includedProperties);
        }

        static const PropertyArea& propertyArea(const T& instance)
        {
            return instance._propertyArea();
        }

        static PropertyArea& propertyArea(T& instance)
        {
            return instance._propertyArea();
        }
    };

    template <typename T>
    struct StructDescriptor<T, true> : StructDescriptor<T, false>
    {
        static_assert(std::is_base_of_v<Struct, T>);

        StructDescriptor(std::string name, uint8_t flags, const property_descriptor_container_t& propertyDescriptor) :
            StructDescriptor<T, false>(std::move(name), flags, propertyDescriptor, sizeof(const StructDescriptor<>*), sizeof(T), alignof(T))
        {
            /* do nothing */
        }
        StructDescriptor(std::string name, uint8_t flags, const property_descriptor_container_t& propertyDescriptor, size_t areaOffset, size_t size, size_t alignment) :
            StructDescriptor<T, false>(std::move(name), flags, propertyDescriptor, areaOffset, size, alignment)
        {
            /* do nothing */
        }
        StructDescriptor(const StructDescriptor& other) = default;
        StructDescriptor(StructDescriptor&& other) = default;
        ~StructDescriptor() = default;

        StructDescriptor& operator = (const StructDescriptor& rhs) = default;
        StructDescriptor& operator = (StructDescriptor&& rhs) = default;

        using StructDescriptor<T, false>::assign;
        using StructDescriptor<T, false>::copy;
        using StructDescriptor<T, false>::merge;
        using StructDescriptor<T, false>::swap;
        using StructDescriptor<T, false>::clear;
        using StructDescriptor<T, false>::equal;
        using StructDescriptor<T, false>::same;
        using StructDescriptor<T, false>::less;
        using StructDescriptor<T, false>::lessEqual;
        using StructDescriptor<T, false>::greater;
        using StructDescriptor<T, false>::greaterEqual;
        using StructDescriptor<T, false>::diffProperties;

        Struct& assign(Struct& instance, const Struct& other, const PropertySet& includedProperties) const override
        {
            return assign(static_cast<T&>(instance), static_cast<const T&>(other), includedProperties);
        }

        Struct& assign(Struct& instance, Struct&& other, const PropertySet& includedProperties) const override
        {
            return assign(static_cast<T&>(instance), static_cast<T&&>(other), includedProperties);
        }

        Struct& copy(Struct& instance, const Struct& other, const PropertySet& includedProperties) const override
        {
            return copy(static_cast<T&>(instance), static_cast<const T&>(other), includedProperties);
        }

        Struct& merge(Struct& instance, const Struct& other, const PropertySet& includedProperties) const override
        {
            return merge(static_cast<T&>(instance), static_cast<const T&>(other), includedProperties);
        }

        void swap(Struct& instance, Struct& other, const PropertySet& includedProperties) const override
        {
            swap(static_cast<T&>(instance), static_cast<T&>(other), includedProperties);
        }

        void clear(Struct& instance, const PropertySet& includedProperties) const override
        {
            clear(static_cast<T&>(instance), includedProperties);
        }

        bool equal(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const override
        {
            return equal(static_cast<const T&>(lhs), static_cast<const T&>(rhs), includedProperties);
        }

        bool same(const Struct& lhs, const Struct& rhs) const override
        {
            return same(static_cast<const T&>(lhs), static_cast<const T&>(rhs));
        }

        bool less(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const override
        {
            return less(static_cast<const T&>(lhs), static_cast<const T&>(rhs), includedProperties);
        }

        bool lessEqual(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const override
        {
            return lessEqual(static_cast<const T&>(lhs), static_cast<const T&>(rhs), includedProperties);
        }

        bool greater(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const override
        {
            return greater(static_cast<const T&>(lhs), static_cast<const T&>(rhs), includedProperties);
        }

        bool greaterEqual(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const override
        {
            return greaterEqual(static_cast<const T&>(lhs), static_cast<const T&>(rhs), includedProperties);
        }

        PropertySet diffProperties(const Struct& instance, const Struct& other, const PropertySet& includedProperties) const override
        {
            return diffProperties(static_cast<const T&>(instance), static_cast<const T&>(other), includedProperties);
        }
    };

    template <typename TDescriptor>
    struct type_category<TDescriptor, std::enable_if_t<std::is_same_v<StructDescriptor<>, TDescriptor>>> : std::integral_constant<Type, Type::Struct> {};

    [[deprecated("only available for backwards compatibility")]]
    inline const StructDescriptor<>* toStructDescriptor(const Descriptor<>* descriptor)
    {
        return descriptor->type() == Type::Struct ? static_cast<const StructDescriptor<>*>(descriptor) : nullptr;
    }
}