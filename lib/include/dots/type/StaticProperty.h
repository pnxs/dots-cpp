// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <type_traits>
#include <optional>
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#include <dots/type/Property.h>
#include <dots/type/StaticPropertyMetadata.h>

namespace dots::type::details
{
    struct dummy_t
    {
        constexpr dummy_t() noexcept
        {
            // explicitly user-provided to avoid zero-initialization
        }
    };

    template <typename  T, bool = std::is_trivially_destructible_v<T>>
    struct storage_t
    {
        union
        {
            dummy_t dummy;
            T value;
        };

        constexpr storage_t() noexcept : dummy{} {}
    };

    template <typename T>
    struct storage_t<T, false>
    {
        union
        {
            dummy_t dummy;
            T value;
        };

        constexpr storage_t() noexcept : dummy{} {}

        storage_t(const storage_t&) = default;
        storage_t(storage_t&& other) = default;

        ~storage_t() noexcept
        {
            // handled in property
        }

        storage_t& operator = (const storage_t& rhs) = default;
        storage_t& operator = (storage_t&& rhs) = default;
    };
}

namespace dots::type
{
    template <typename T, typename Derived>
    struct StaticProperty : Property<T, Derived>
    {
        template <typename... Args, std::enable_if_t<!std::is_arithmetic_v<T> && sizeof...(Args) >= 1 && std::is_constructible_v<T, Args...>, int> = 0>
        StaticProperty(Args&&... args)
        {
            StaticProperty<T, Derived>::emplace(std::forward<Args>(args)...);
        }

        template <typename T_ = T, std::enable_if_t<std::is_arithmetic_v<T_>, int> = 0>
        StaticProperty(T value)
        {
            StaticProperty<T, Derived>::emplace(value);
        }

        template <typename D, std::enable_if_t<!std::is_same_v<D, Derived>, int> = 0>
        StaticProperty(const Property<T, D>& other)
        {
            Property<T, Derived>::assign(other);
        }

        template <typename D, std::enable_if_t<!std::is_same_v<D, Derived>, int> = 0>
        StaticProperty(Property<T, D>&& other)
        {
            Property<T, Derived>::assign(std::move(other));
        }

        template <typename D, std::enable_if_t<!std::is_same_v<D, Derived>, int> = 0>
        Derived& operator = (const Property<T, D>& rhs)
        {
            return Property<T, Derived>::assign(rhs);
        }

        template <typename D, std::enable_if_t<!std::is_same_v<D, Derived>, int> = 0>
        Derived& operator = (Property<T, D>&& rhs)
        {
            return Property<T, Derived>::assign(std::move(rhs));
        }

        using Property<T, Derived>::operator=;

        static constexpr std::string_view Name()
        {
            return Derived::Metadata.name();
        }

        static constexpr uint32_t Tag()
        {
            return Derived::Metadata.tag();
        }

        static constexpr bool IsKey()
        {
            return Derived::Metadata.isKey();
        }

        static constexpr PropertyOffset Offset()
        {
            return Derived::Metadata.offset();
        }

        static constexpr PropertySet Set()
        {
            return Derived::Metadata.set();
        }

        static constexpr bool IsPartOf(PropertySet propertySet)
        {
            return Set() <= propertySet;
        }

        static const PropertyDescriptor& InitDescriptor()
        {
            if (M_descriptorStorage == std::nullopt)
            {
                M_descriptorStorage.emplace(type::Descriptor<T>::Instance(), Name().data(), Tag(), IsKey(), Offset());
            }

            return *M_descriptorStorage;
        }

        inline static const PropertyDescriptor& Descriptor = InitDescriptor();

    protected:

        StaticProperty() = default;

        StaticProperty(const StaticProperty& other) : Property<T, Derived>()
        {
            *this = other;
        }

        StaticProperty(StaticProperty&& other)
        {
            *this = std::move(other);
        }

        ~StaticProperty()
        {
            Property<T, Derived>::reset();
        }

        StaticProperty& operator = (const StaticProperty& rhs)
        {
            Property<T, Derived>::assign(static_cast<const Derived&>(rhs));
            return *this;
        }

        StaticProperty& operator = (StaticProperty&& rhs)
        {
            Property<T, Derived>::assign(static_cast<Derived&&>(rhs));
            return *this;
        }

    private:

        friend struct Property<T, Derived>;

        PropertySet validProperties() const
        {
            return PropertyArea::GetArea(static_cast<const Derived&>(*this)).validProperties();
        }

        PropertySet& validProperties()
        {
            return PropertyArea::GetArea(static_cast<Derived&>(*this)).validProperties();
        }

        static const PropertyDescriptor& derivedDescriptor()
        {
            return Derived::Descriptor;
        }

        const T& derivedStorage() const
        {
            return m_storage.value;
        }

        bool derivedIsValid() const
        {
            return Set() <= validProperties();
        }

        void derivedSetValid()
        {
            validProperties() += derivedDescriptor().set();
        }

        void derivedSetInvalid()
        {
            validProperties() -= derivedDescriptor().set();
        }

        inline static std::optional<PropertyDescriptor> M_descriptorStorage;

        details::storage_t<T> m_storage;
    };
}

#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
