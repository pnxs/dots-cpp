// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <type_traits>
#include <dots/type/PropertyContainer.h>
#include <dots/type/StructDescriptor.h>

namespace dots::type
{
    struct Struct : PropertyContainer<Struct>
    {
        explicit Struct(const StructDescriptor& descriptor) :
            _desc(&descriptor)
        {
            /* do nothing */
        }

        const StructDescriptor& _descriptor() const
        {
            return *_desc;
        }

        bool _usesDynamicMemory() const
        {
            return _desc->usesDynamicMemory();
        }
        size_t _dynamicMemoryUsage() const
        {
            return _desc->dynamicMemoryUsage(Typeless::From(*this));
        }

        size_t _staticMemoryUsage() const
        {
            return _desc->size();
        }

        size_t _totalMemoryUsage() const;

        PropertySet _properties() const
        {
            return _desc->properties();
        }

        PropertySet _keyProperties() const
        {
            return _desc->keyProperties();
        }

        Struct& _assign(const Struct& other, PropertySet includedProperties = PropertySet::All)
        {
            return _desc->assign(*this, other, includedProperties);
        }

        Struct& _assign(Struct&& other, PropertySet includedProperties = PropertySet::All)
        {
            return _desc->assign(*this, std::move(other), includedProperties);
        }

        Struct& _copy(const Struct& other, PropertySet includedProperties = PropertySet::All)
        {
            return _desc->copy(*this, other, includedProperties);
        }

        Struct& _merge(const Struct& other, PropertySet includedProperties = PropertySet::All)
        {
            return _desc->merge(*this, other, includedProperties);
        }

        void _swap(Struct& other, PropertySet includedProperties = PropertySet::All)
        {
            return _desc->swap(*this, other, includedProperties);
        }

        void _clear(PropertySet includedProperties = PropertySet::All)
        {
            _desc->clear(*this, includedProperties);
        }

        bool _equal(const Struct& rhs, PropertySet includedProperties = PropertySet::All) const
        {
            return _desc->equal(*this, rhs, includedProperties);
        }

        bool _same(const Struct& rhs) const
        {
            return _desc->same(*this, rhs);
        }

        bool _less(const Struct& rhs, PropertySet includedProperties = PropertySet::All) const
        {
            return _desc->less(*this, rhs, includedProperties);
        }

        bool _lessEqual(const Struct& rhs, PropertySet includedProperties = PropertySet::All) const
        {
            return _desc->lessEqual(*this, rhs, includedProperties);
        }

        bool _greater(const Struct& rhs, PropertySet includedProperties = PropertySet::All) const
        {
            return _desc->greater(*this, rhs, includedProperties);
        }

        bool _greaterEqual(const Struct& rhs, PropertySet includedProperties = PropertySet::All) const
        {
            return _desc->greaterEqual(*this, rhs, includedProperties);
        }

        PropertySet _diffProperties(const Struct& other, PropertySet includedProperties = PropertySet::All) const
        {
            return _desc->diffProperties(*this, other, includedProperties);
        }

        template <bool AllowSubset = true>
        bool _hasProperties(PropertySet properties) const
        {
            if constexpr (AllowSubset)
            {
                return properties <= _validProperties();
            }
            else
            {
                return properties == _validProperties();
            }
        }

        template <bool AllowSubset = true>
        void _assertHasProperties(PropertySet expectedProperties) const
        {
            PropertySet actualProperties = _validProperties();

            if (!_hasProperties<AllowSubset>(expectedProperties))
            {
                auto to_property_list = [](const StructDescriptor& descriptor, PropertySet properties)
                {
                    std::string propertyList;

                    for (const PropertyDescriptor& propertyDescriptor : descriptor.propertyDescriptors(properties))
                    {
                        propertyList += propertyDescriptor.name();
                        propertyList += ", ";
                    }

                    if (!propertyList.empty())
                    {
                        propertyList.resize(propertyList.size() - 2);
                    }

                    return propertyList;
                };

                if constexpr (AllowSubset)
                {
                    throw std::logic_error{ _desc->name() + " instance is missing expected properties: " + to_property_list(*_desc, expectedProperties - actualProperties) };
                }
                else
                {
                    throw std::logic_error{ _desc->name() + " instance does not have expected exact properties: " + to_property_list(*_desc, expectedProperties) + ", but instead has " + to_property_list(*_desc, actualProperties) };
                }
            }
        }

        template <typename TDescriptor>
        bool _is(TDescriptor&& descriptor) const
        {
            static_assert(!std::is_rvalue_reference_v<TDescriptor>);
            static_assert(std::is_base_of_v<StructDescriptor, std::remove_pointer_t<std::decay_t<TDescriptor>>>);

            return &ToRef(std::forward<TDescriptor>(descriptor)) == _desc;
        }

        template <typename... Descriptors>
        bool _isAny(Descriptors&&... descriptors) const
        {
            static_assert(sizeof...(Descriptors) > 0);
            return (_is(std::forward<Descriptors>(descriptors)) || ...);
        }

        template <typename T>
        bool _is() const
        {
            static_assert(std::is_base_of_v<Struct, T>, "T has to be a sub-class of Struct");
            return _is(T::_Descriptor());
        }

        template <typename... Ts>
        bool _isAny() const
        {
            static_assert(std::conjunction_v<std::is_base_of<Struct, Ts>...>);
            return _isAny(Ts::_Descriptor()...);
        }

        template <typename TDescriptor>
        void _assertIs(TDescriptor&& descriptor) const
        {
            if (!_is(std::forward<TDescriptor>(descriptor)))
            {
                throw std::logic_error{ _desc->name() + " instance does not have expected type: " + ToRef(std::forward<TDescriptor>(descriptor)).name() };
            }
        }

        template <typename... Descriptors>
        void _assertIsAny(Descriptors&&... descriptors) const
        {
            if (!_isAny(std::forward<Descriptors>(descriptors)...))
            {
                std::string expectedTypes = ((ToRef(std::forward<Descriptors>(descriptors)).name() + ", ") + ...);
                expectedTypes.resize(expectedTypes.size() - 2);

                throw std::logic_error{ _desc->name() + " instance does not have any of expected types: " + expectedTypes };
            }
        }

        template <typename T>
        void _assertIs() const
        {
            static_assert(std::is_base_of_v<Struct, T>, "T has to be a sub-class of Struct");
            return _assertIs(T::_Descriptor());
        }

        template <typename... Ts>
        void _assertIsAny() const
        {
            static_assert(std::conjunction_v<std::is_base_of<Struct, Ts>...>);
            return _assertIsAny(Ts::_Descriptor()...);
        }

        template <typename T>
        const T* _as() const
        {
            static_assert(std::is_base_of_v<Struct, T>, "T has to be a sub-class of Struct");
            return _is<T>() ? static_cast<const T*>(this) : nullptr;
        }

        template <typename T>
        T* _as()
        {
            return const_cast<T*>(std::as_const(*this)._as<T>());
        }

        template <typename T, bool Safe = false>
        const T& _to() const
        {
            static_assert(std::is_base_of_v<Struct, T>, "T has to be a sub-class of Struct");

            if constexpr (Safe)
            {
                if (!_is<T>())
                {
                    throw std::logic_error{ std::string{ "type mismatch in safe Struct conversion: expected " } + _desc->name().data() + " but got " + T::_Descriptor().name() };
                }
            }

            return static_cast<const T&>(*this);
        }

        template <typename T, bool Safe = false>
        T& _to()
        {
            return const_cast<T&>(std::as_const(*this)._to<T, Safe>());
        }

    private:

        friend struct PropertyContainer<Struct>;

        const PropertyArea& derivedPropertyArea() const
        {
            return _desc->propertyArea(*this);
        }

        PropertyArea& derivedPropertyArea()
        {
            return _desc->propertyArea(*this);
        }

        const property_descriptor_container_t& derivedPropertyDescriptors() const
        {
            return _desc->propertyDescriptors();
        }

        const std::vector<PropertyPath>& derivedPropertyPaths() const
        {
            return _desc->propertyPaths();
        }

        template <typename T>
        static decltype(auto) ToRef(T&& t)
        {
            if constexpr (std::is_pointer_v<std::decay_t<T>>)
            {
                return *t;
            }
            else
            {
                return t;
            }
        }

        const StructDescriptor* _desc;
    };

    template <typename T, std::enable_if_t<std::is_base_of_v<Struct, std::decay_t<T>>, int> = 0>
    bool operator == (const T& lhs, const T& rhs)
    {
        return lhs._equal(rhs);
    }

    template <typename T, std::enable_if_t<std::is_base_of_v<Struct, std::decay_t<T>>, int> = 0>
    bool operator != (const T& lhs, const T& rhs)
    {
        return !(lhs == rhs);
    }

    template <typename T, std::enable_if_t<std::is_base_of_v<Struct, std::decay_t<T>>, int> = 0>
    bool operator < (const T& lhs, const T& rhs)
    {
        return lhs._less(rhs);
    }

    template <typename T, std::enable_if_t<std::is_base_of_v<Struct, std::decay_t<T>>, int> = 0>
    bool operator <= (const T& lhs, const T& rhs)
    {
        return !(lhs > rhs);
    }

    template <typename T, std::enable_if_t<std::is_base_of_v<Struct, std::decay_t<T>>, int> = 0>
    bool operator > (const T& lhs, const T& rhs)
    {
        return rhs < lhs;
    }

    template <typename T, std::enable_if_t<std::is_base_of_v<Struct, std::decay_t<T>>, int> = 0>
    bool operator >= (const T& lhs, const T& rhs)
    {
        return !(lhs < rhs);
    }
}
