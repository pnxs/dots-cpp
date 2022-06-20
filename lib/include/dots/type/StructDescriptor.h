#pragma once
#include <dots/type/Descriptor.h>
#include <dots/type/StaticDescriptor.h>
#include <dots/type/Property.h>
#include <dots/type/PropertyPath.h>

namespace dots::type
{
    struct Struct;

    struct StructDescriptor : StaticDescriptor
    {
        static const uint8_t Uncached      = 0b0000'0000;
        static const uint8_t Cached        = 0b0000'0001;
        static const uint8_t Internal      = 0b0000'0010;
        static const uint8_t Persistent    = 0b0000'0100;
        static const uint8_t Cleanup       = 0b0000'1000;
        static const uint8_t Local         = 0b0001'0000;
        static const uint8_t SubstructOnly = 0b0010'0000;

        StructDescriptor(key_t key, std::string name, uint8_t flags, const property_descriptor_container_t& propertyDescriptors, size_t areaOffset, size_t size, size_t alignment);
        StructDescriptor(const StructDescriptor& other) = delete;
        StructDescriptor(StructDescriptor&& other) = delete;
        ~StructDescriptor() override = default;

        StructDescriptor& operator = (const StructDescriptor& rhs) = delete;
        StructDescriptor& operator = (StructDescriptor&& rhs) = delete;

        using StaticDescriptor::construct;
        using StaticDescriptor::constructInPlace;
        using StaticDescriptor::destruct;
        using StaticDescriptor::assign;
        using StaticDescriptor::swap;
        using StaticDescriptor::equal;
        using StaticDescriptor::less;
        using StaticDescriptor::lessEqual;
        using StaticDescriptor::greater;
        using StaticDescriptor::greaterEqual;
        using StaticDescriptor::dynamicMemoryUsage;

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

        Typeless& assign(Typeless& lhs) const override;
        Typeless& assign(Typeless& lhs, const Typeless& rhs) const override;
        Typeless& assign(Typeless& lhs, Typeless&& rhs) const override;
        void swap(Typeless& value, Typeless& other) const override;

        bool equal(const Typeless& lhs, const Typeless& rhs) const override;
        bool less(const Typeless& lhs, const Typeless& rhs) const override;

        size_t areaOffset() const
        {
            return m_areaOffset;
        }

        size_t numSubStructs() const
        {
            return m_numSubStructs;
        }

        bool usesDynamicMemory() const override;
        size_t dynamicMemoryUsage(const Typeless& instance) const override;
        size_t dynamicMemoryUsage(const Struct& instance) const;

        virtual Struct& assign(Struct& instance, const Struct& other, PropertySet includedProperties) const;
        virtual Struct& assign(Struct& instance, Struct&& other, PropertySet includedProperties) const;
        virtual Struct& copy(Struct& instance, const Struct& other, PropertySet includedProperties) const;
        virtual Struct& merge(Struct& instance, const Struct& other, PropertySet includedProperties) const;
        virtual void swap(Struct& instance, Struct& other, PropertySet includedProperties) const;
        virtual void clear(Struct& instance, PropertySet includedProperties) const;

        virtual bool equal(const Struct& lhs, const Struct& rhs, PropertySet includedProperties) const;
        virtual bool same(const Struct& lhs, const Struct& rhs) const;

        virtual bool less(const Struct& lhs, const Struct& rhs, PropertySet includedProperties) const;
        bool lessEqual(const Struct& lhs, const Struct& rhs, PropertySet includedProperties) const;
        bool greater(const Struct& lhs, const Struct& rhs, PropertySet includedProperties) const;
        bool greaterEqual(const Struct& lhs, const Struct& rhs, PropertySet includedProperties) const;

        virtual PropertySet diffProperties(const Struct& instance, const Struct& other, PropertySet includedProperties) const;

        virtual const PropertyArea& propertyArea(const Struct& instance) const = 0;
        virtual PropertyArea& propertyArea(Struct& instance) const = 0;

        uint8_t flags() const
        {
            return m_flags;
        }

        bool cached() const
        {
            return static_cast<bool>(m_flags & Cached);
        }

        bool cleanup() const
        {
            return static_cast<bool>(m_flags & Cleanup);
        }

        bool local() const
        {
            return static_cast<bool>(m_flags & Local);
        }

        bool persistent() const
        {
            return static_cast<bool>(m_flags & Persistent);
        }

        bool internal() const
        {
            return static_cast<bool>(m_flags & Internal);
        }

        bool substructOnly() const
        {
            return static_cast<bool>(m_flags & SubstructOnly);
        }

        const property_descriptor_container_t& propertyDescriptors() const
        {
            return m_propertyDescriptors;
        }

        partial_property_descriptor_container_t propertyDescriptors(PropertySet properties) const;
        property_descriptor_container_t& propertyDescriptors();
        const std::vector<PropertyPath>& propertyPaths() const;

        PropertySet properties() const
        {
            return m_properties;
        }

        PropertySet keyProperties() const
        {
            return m_keyProperties;
        }

        PropertySet dynamicMemoryProperties() const
        {
            return m_dynamicMemoryProperties;
        }

        template <typename T, std::enable_if_t<!std::is_same_v<T, Struct>, int> = 0>
        static T& assign(T& instance, const T& other, PropertySet includedProperties)
        {
            return instance._assign(other, includedProperties);
        }

        template <typename T, std::enable_if_t<!std::is_same_v<T, Struct>, int> = 0>
        static T& assign(T& instance, T&& other, PropertySet includedProperties)
        {
            return instance._assign(std::move(other), includedProperties);
        }

        template <typename T, std::enable_if_t<!std::is_same_v<T, Struct>, int> = 0>
        static T& copy(T& instance, const T& other, PropertySet includedProperties)
        {
            return instance._copy(other, includedProperties);
        }

        template <typename T, std::enable_if_t<!std::is_same_v<T, Struct>, int> = 0>
        static T& merge(T& instance, const T& other, PropertySet includedProperties)
        {
            return instance._merge(other, includedProperties);
        }

        template <typename T, std::enable_if_t<!std::is_same_v<T, Struct>, int> = 0>
        static void swap(T& instance, T& other, PropertySet includedProperties)
        {
            instance._swap(other, includedProperties);
        }

        template <typename T, std::enable_if_t<!std::is_same_v<T, Struct>, int> = 0>
        static void clear(T& instance, PropertySet includedProperties)
        {
            instance._clear(includedProperties);
        }

        template <typename T, std::enable_if_t<!std::is_same_v<T, Struct>, int> = 0>
        static bool equal(const T& lhs, const T& rhs, PropertySet includedProperties)
        {
            return lhs._equal(rhs, includedProperties);
        }

        template <typename T, std::enable_if_t<!std::is_same_v<T, Struct>, int> = 0>
        static bool same(const T& lhs, const T& rhs)
        {
            return lhs._same(rhs);
        }

        template <typename T, std::enable_if_t<!std::is_same_v<T, Struct>, int> = 0>
        static bool less(const T& lhs, const T& rhs, PropertySet includedProperties)
        {
            return lhs._less(rhs, includedProperties);
        }

        template <typename T, std::enable_if_t<!std::is_same_v<T, Struct>, int> = 0>
        static bool lessEqual(const T& lhs, const T& rhs, PropertySet includedProperties)
        {
            return lhs._lessEqual(rhs, includedProperties);
        }

        template <typename T, std::enable_if_t<!std::is_same_v<T, Struct>, int> = 0>
        static bool greater(const T& lhs, const T& rhs, PropertySet includedProperties)
        {
            return lhs._greater(rhs, includedProperties);
        }

        template <typename T, std::enable_if_t<!std::is_same_v<T, Struct>, int> = 0>
        static bool greaterEqual(const T& lhs, const T& rhs, PropertySet includedProperties)
        {
            return lhs._greaterEqual(rhs, includedProperties);
        }

        template <typename T, std::enable_if_t<!std::is_same_v<T, Struct>, int> = 0>
        static PropertySet diffProperties(const T& instance, const T& other, PropertySet includedProperties)
        {
            return instance._diffProperties(other, includedProperties);
        }

        template <typename T, std::enable_if_t<!std::is_same_v<T, Struct>, int> = 0>
        static const PropertyArea& propertyArea(const T& instance)
        {
            return instance._propertyArea();
        }

        template <typename T, std::enable_if_t<!std::is_same_v<T, Struct>, int> = 0>
        static PropertyArea& propertyArea(T& instance)
        {
            return instance._propertyArea();
        }

    private:

        uint8_t m_flags;
        property_descriptor_container_t m_propertyDescriptors;
        size_t m_areaOffset;
        PropertySet m_properties;
        PropertySet m_keyProperties;
        size_t m_numSubStructs;
        PropertySet m_dynamicMemoryProperties;
        mutable std::vector<PropertyPath> m_propertyPaths;
    };

    template <typename TDescriptor>
    struct type_category<TDescriptor, std::enable_if_t<std::is_same_v<StructDescriptor, TDescriptor>>> : std::integral_constant<Type, Type::Struct> {};
}
