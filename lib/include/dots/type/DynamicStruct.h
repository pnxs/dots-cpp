// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <string_view>
#include <dots/type/Struct.h>

namespace dots
{
    template<typename T>
    struct Event;
}

namespace dots::type
{
    template <typename T>
    struct DynamicPropertyInitializer
    {
        DynamicPropertyInitializer(std::string_view name, const T& value) : name(name), value(value) {}
        DynamicPropertyInitializer(std::string_view name, T&& value) : name(name), value(std::move(value)) {}

        std::string_view name;
        T value;
    };

    template <typename T>
    struct is_dynamic_property_initializer : std::false_type {};

    template <typename T>
    struct is_dynamic_property_initializer<DynamicPropertyInitializer<T>> : std::true_type {};

    template <typename T>
    using is_dynamic_property_initializer_t = typename is_dynamic_property_initializer<T>::type;

    template <typename T>
    constexpr bool is_dynamic_property_initializer_v = is_dynamic_property_initializer_t<T>::value;

    struct DynamicStruct;

    template <>
    struct Descriptor<DynamicStruct>;

    struct DynamicStruct : Struct
    {
        using Cbd = Event<DynamicStruct>;
        template <typename T>
        using property_i = DynamicPropertyInitializer<T>;

        DynamicStruct(const Descriptor<DynamicStruct>& descriptor);
        DynamicStruct(const Descriptor<DynamicStruct>& descriptor, PropertyArea* propertyArea);

        template <typename... DynamicPropertyInitializers, std::enable_if_t<sizeof...(DynamicPropertyInitializers) >= 1 && std::conjunction_v<is_dynamic_property_initializer_t<std::remove_pointer_t<std::decay_t<DynamicPropertyInitializers>>>...>, int> = 0>
        DynamicStruct(const Descriptor<DynamicStruct>& descriptor, DynamicPropertyInitializers&&... dynamicPropertyInitializers) :
            DynamicStruct(descriptor)
        {
            (this->operator[](dynamicPropertyInitializers.name)->emplace(Typeless::From(std::forward<decltype(dynamicPropertyInitializers)>(dynamicPropertyInitializers).value)), ...);
        }

        template <typename... DynamicPropertyInitializers, std::enable_if_t<sizeof...(DynamicPropertyInitializers) >= 1 && std::conjunction_v<is_dynamic_property_initializer_t<std::remove_pointer_t<std::decay_t<DynamicPropertyInitializers>>>...>, int> = 0>
        DynamicStruct(const Descriptor<DynamicStruct>& descriptor, PropertyArea* propertyArea, DynamicPropertyInitializers&&... dynamicPropertyInitializers) :
            DynamicStruct(descriptor, propertyArea)
        {
            (this->operator[](dynamicPropertyInitializers.name)->emplace(Typeless::From(std::forward<decltype(dynamicPropertyInitializers)>(dynamicPropertyInitializers).value)), ...);
        }

        DynamicStruct(const DynamicStruct& other);
        DynamicStruct(DynamicStruct&& other);

        ~DynamicStruct()
        {
            if (m_propertyArea != nullptr)
            {
                _clear();
            }
        }

        DynamicStruct& operator = (const DynamicStruct& rhs)
        {
            _assign(rhs);
            return *this;
        }

        DynamicStruct& operator = (DynamicStruct&& rhs)
        {
            _assign(std::move(rhs));
            return *this;
        }

        using Struct::_assign;
        using Struct::_copy;
        using Struct::_merge;
        using Struct::_swap;
        using Struct::_clear;

        using Struct::_equal;
        using Struct::_same;

        using Struct::_less;
        using Struct::_lessEqual;
        using Struct::_greater;
        using Struct::_greaterEqual;

        using Struct::_diffProperties;

        DynamicStruct& _assign(const DynamicStruct& other, PropertySet includedProperties = PropertySet::All)
        {
            return static_cast<DynamicStruct&>(Struct::_assign(other, includedProperties));
        }

        DynamicStruct& _assign(DynamicStruct&& other, PropertySet includedProperties = PropertySet::All)
        {
            return static_cast<DynamicStruct&>(Struct::_assign(std::move(other), includedProperties));
        }

        DynamicStruct& _copy(const DynamicStruct& other, PropertySet includedProperties = PropertySet::All)
        {
            return static_cast<DynamicStruct&>(Struct::_copy(other, includedProperties));
        }

        DynamicStruct& _merge(const DynamicStruct& other, PropertySet includedProperties = PropertySet::All)
        {
            return static_cast<DynamicStruct&>(Struct::_merge(other, includedProperties));
        }

        void _swap(DynamicStruct& other, PropertySet includedProperties = PropertySet::All)
        {
            Struct::_swap(other, includedProperties);
        }

        void _clear(PropertySet includedProperties = PropertySet::All)
        {
            Struct::_clear(includedProperties);
        }

        bool _equal(const DynamicStruct& rhs, PropertySet includedProperties = PropertySet::All) const
        {
            return Struct::_equal(rhs, includedProperties);
        }

        bool _same(const DynamicStruct& rhs) const
        {
            return Struct::_same(rhs);
        }

        bool _lessEqual(const DynamicStruct& rhs, PropertySet includedProperties = PropertySet::All) const
        {
            return Struct::_lessEqual(rhs, includedProperties);
        }

        bool _greater(const DynamicStruct& rhs, PropertySet includedProperties = PropertySet::All) const
        {
            return Struct::_greater(rhs, includedProperties);
        }

        bool _greaterEqual(const DynamicStruct& rhs, PropertySet includedProperties = PropertySet::All) const
        {
            return Struct::_greaterEqual(rhs, includedProperties);
        }

        PropertySet _diffProperties(const DynamicStruct& other, PropertySet includedProperties = PropertySet::All) const
        {
            return Struct::_diffProperties(other, includedProperties);
        }

        const PropertyArea& _propertyArea() const
        {
            return *m_propertyArea;
        }

        PropertyArea& _propertyArea()
        {
            return *m_propertyArea;
        }

    private:

        template <typename T>
        using strip_t = std::remove_pointer_t<std::decay_t<T>>;

        using Struct::_propertyArea;

        std::unique_ptr<PropertyArea> m_propertyAreaStorage;
        PropertyArea* m_propertyArea;
    };

    template <>
    struct Descriptor<DynamicStruct> : StructDescriptor
    {
        static constexpr bool IsDynamic = true;

        Descriptor(key_t key, std::string name, uint8_t flags, const property_descriptor_container_t& propertyDescriptors, size_t size) :
            StructDescriptor(key, std::move(name), flags, propertyDescriptors, sizeof(DynamicStruct), size, alignof(DynamicStruct))
        {
            /* do nothing */
        }
        Descriptor(const Descriptor& other) = delete;
        Descriptor(Descriptor&& other) = delete;
        ~Descriptor() override = default;

        Descriptor& operator = (const Descriptor& rhs) = delete;
        Descriptor& operator = (Descriptor&& rhs) = delete;

        using StaticDescriptor::construct;
        using StaticDescriptor::constructInPlace;
        using StaticDescriptor::assign;

        DynamicStruct& construct(DynamicStruct& value) const
        {
            return construct(value, *this);
        }

        DynamicStruct& construct(DynamicStruct& value, const DynamicStruct& other) const
        {
            DynamicStruct& instance = construct(value);
            return assign(instance, other);
        }

        DynamicStruct& construct(DynamicStruct& value, DynamicStruct&& other) const
        {
            DynamicStruct& instance = construct(value);
            return assign(instance, std::move(other));
        }

        Typeless& construct(Typeless& value) const override
        {
            return Typeless::From(construct(value.to<DynamicStruct>()));
        }

        Typeless& construct(Typeless& value, const Typeless& other) const override
        {
            return Typeless::From(construct(value.to<DynamicStruct>(), other.to<DynamicStruct>()));
        }

        Typeless& construct(Typeless& value, Typeless&& other) const override
        {
            return Typeless::From(construct(value.to<DynamicStruct>(), std::move(other.to<DynamicStruct>())));
        }

        DynamicStruct& constructInPlace(DynamicStruct& value) const
        {
            return construct(value, *this, reinterpret_cast<PropertyArea*>(&Typeless::From(value).to<std::byte>() + sizeof(DynamicStruct)));
        }

        DynamicStruct& constructInPlace(DynamicStruct& value, const DynamicStruct& other) const
        {
            DynamicStruct& instance = constructInPlace(value);
            return assign(instance, other);
        }

        DynamicStruct& constructInPlace(DynamicStruct& value, DynamicStruct&& other) const
        {
            DynamicStruct& instance = constructInPlace(value);
            return assign(instance, std::move(other));
        }

        Typeless& constructInPlace(Typeless& value) const override
        {
            return Typeless::From(constructInPlace(value.to<DynamicStruct>()));
        }

        Typeless& constructInPlace(Typeless& value, const Typeless& other) const override
        {
            return Typeless::From(constructInPlace(value.to<DynamicStruct>(), other.to<DynamicStruct>()));
        }

        Typeless& constructInPlace(Typeless& value, Typeless&& other) const override
        {
            return Typeless::From(constructInPlace(value.to<DynamicStruct>(), std::move(other.to<DynamicStruct>())));
        }

        const PropertyArea& propertyArea(const Struct& instance) const override
        {
            return StructDescriptor::propertyArea(static_cast<const DynamicStruct&>(instance));
        }

        PropertyArea& propertyArea(Struct& instance) const override
        {
            return StructDescriptor::propertyArea(static_cast<DynamicStruct&>(instance));
        }

        static const PropertyArea& propertyArea(const DynamicStruct& instance)
        {
            return instance._propertyArea();
        }

        static PropertyArea& propertyArea(DynamicStruct& instance)
        {
            return instance._propertyArea();
        }
    };

    inline DynamicStruct::DynamicStruct(const DynamicStruct& other) :
        DynamicStruct(other._descriptor().to<Descriptor<DynamicStruct>, true>())
    {
        *this = other;
    }

    inline DynamicStruct::DynamicStruct(DynamicStruct&& other) :
        DynamicStruct(other._descriptor().to<Descriptor<DynamicStruct>, true>())
    {
        *this = std::move(other);
    }
}
