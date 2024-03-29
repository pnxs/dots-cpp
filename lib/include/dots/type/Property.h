// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <type_traits>
#include <cstddef>
#include <dots/type/PropertyArea.h>
#include <dots/type/PropertyDescriptor.h>

namespace dots
{
    struct invalid_t{};
    inline constexpr invalid_t invalid;
}

namespace dots::type
{
    namespace details
    {
        struct property_tag {};
    }

    template <typename T>
    using is_property = std::is_base_of<details::property_tag, std::decay_t<T>>;

    template <typename T>
    using is_property_t = typename is_property<T>::type;

    template <typename T>
    constexpr bool is_property_v = is_property_t<T>::value;

    template <typename T, typename Derived>
    struct Property : details::property_tag
    {
        static_assert(std::conjunction_v<std::negation<std::is_pointer<T>>, std::negation<std::is_reference<T>>>);
        using value_t = T;
        static constexpr bool IsTypeless = std::is_same_v<T, Typeless>;

        template <typename Rhs, std::enable_if_t<!std::is_arithmetic_v<T> && (std::is_same_v<Rhs, Typeless> || std::is_constructible_v<T, Rhs>), int> = 0>
        Derived& operator = (Rhs&& rhs)
        {
            return assign(std::forward<Rhs>(rhs));
        }

        template <typename T_ = T, std::enable_if_t<std::is_arithmetic_v<T_>, int> = 0>
        Derived& operator = (T rhs)
        {
            return assign(rhs);
        }

        Derived& operator = (invalid_t)
        {
            reset();
            return static_cast<Derived&>(*this);
        }

        T& operator * ()
        {
            return value();
        }

        const T& operator * () const
        {
            return value();
        }

        T* operator -> ()
        {
            return &value();
        }

        const T* operator -> () const
        {
            return &value();
        }

        bool isValid() const
        {
            return static_cast<const Derived&>(*this).derivedIsValid();
        }

        template <typename Rhs, std::enable_if_t<std::is_same_v<Rhs, Typeless> || std::is_constructible_v<T, Rhs>, int> = 0>
        Derived& assign(Rhs&& rhs)
        {
            emplace(std::forward<Rhs>(rhs));
            return static_cast<Derived&>(*this);
        }

        template <typename D>
        Derived& assign(const Property<T, D>& rhs)
        {
            if (rhs.isValid())
            {
                emplace(rhs.storage());
            }
            else
            {
                reset();
            }

            return static_cast<Derived&>(*this);
        }

        template <typename D>
        Derived& assign(Property<T, D>&& rhs)
        {
            if (rhs.isValid())
            {
                emplace(std::move(rhs.storage()));
                rhs.reset();
            }
            else
            {
                reset();
            }

            return static_cast<Derived&>(*this);
        }

        template <typename... Args, std::enable_if_t<!std::disjunction_v<is_property<Args>...>, int> = 0>
        T& emplace(Args&&... args)
        {
            static_assert(!IsTypeless || sizeof...(Args) <= 1, "typeless construct or assignment only supports a single argument");

            if (isValid())
            {
                descriptor().valueDescriptor().assign(storage(), std::forward<Args>(args)...);
            }
            else
            {
                static_cast<Derived&>(*this).derivedSetValid();
                descriptor().valueDescriptor().constructInPlace(storage(), std::forward<Args>(args)...);
            }

            return storage();
        }

        void reset()
        {
            if (isValid())
            {
                descriptor().valueDescriptor().destruct(storage());
                static_cast<Derived&>(*this).derivedSetInvalid();
            }
        }

        const T& value() const
        {
            if (!isValid())
            {
                throw std::runtime_error{ "property is expected to be valid but it is not: " + descriptor().name() };
            }

            return storage();
        }

        T& value()
        {
            return const_cast<T&>(std::as_const(*this).value());
        }

        template <typename... Args>
        T valueOrDefault(Args&&... args) const
        {
            if (isValid())
            {
                return storage();
            }
            else
            {
                return T(std::forward<Args>(args)...);
            }
        }

        template <typename... Args>
        T& valueOrEmplace(Args&&... args)
        {
            if (isValid())
            {
                return storage();
            }
            else
            {
                return emplace(std::forward<Args>(args)...);
            }
        }

        template <typename D>
        void swap(Property<T, D>& other)
        {
            if (isValid())
            {
                if (other.isValid())
                {
                    return descriptor().valueDescriptor().swap(storage(), other.storage());
                }
                else
                {
                    other.emplace(std::move(storage()));
                    reset();
                }
            }
            else if (other.isValid())
            {
                emplace(std::move(other.storage()));
                other.reset();
            }
        }

        constexpr const PropertyDescriptor& descriptor() const
        {
            return static_cast<const Derived&>(*this).derivedDescriptor();
        }

        constexpr bool isPartOf(PropertySet propertySet) const
        {
            return descriptor().set() <= propertySet;
        }

        constexpr const T& storage() const
        {
            return static_cast<const Derived&>(*this).derivedStorage();
        }

        constexpr T& storage()
        {
            return const_cast<T&>(std::as_const(*this).storage());
        }

    protected:

        constexpr Property() = default;
        constexpr Property(const Property& other) = default;
        constexpr Property(Property&& other) = default;
        ~Property() = default;

        constexpr Property& operator = (const Property& rhs) = default;
        constexpr Property& operator = (Property&& rhs) = default;
    };

    template <typename Lhs, typename Rhs, std::enable_if_t<std::disjunction_v<is_property<Lhs>, is_property<Rhs>>, int> = 0>
    bool operator == (const Lhs& lhs, const Rhs& rhs)
    {
        constexpr bool LhsIsProperty = is_property_v<Lhs>;
        constexpr bool RhsIsProperty = is_property_v<Rhs>;
        constexpr bool RhsIsInvalidType = std::is_same_v<std::decay_t<Rhs>, dots::invalid_t>;

        if constexpr (!LhsIsProperty)
        {
            return rhs == lhs;
        }
        else if constexpr (/*LhsIsProperty && */RhsIsProperty)
        {
            return rhs.isValid() ? lhs == rhs.storage() : lhs == dots::invalid;
        }
        else if constexpr (/*LhsIsProperty && */RhsIsInvalidType)
        {
            return !lhs.isValid();
        }
        else/* if constexpr (LhsIsProperty && !RhsIsInvalidType)*/
        {
            return lhs.isValid() ? lhs.descriptor().valueDescriptor().equal(lhs.storage(), rhs) : false;
        }
    }

    template <typename Lhs, typename Rhs, std::enable_if_t<std::disjunction_v<is_property<Lhs>, is_property<Rhs>>, int> = 0>
    bool operator != (const Lhs& lhs, const Rhs& rhs)
    {
        return !(lhs == rhs);
    }

    template <typename Lhs, typename Rhs, std::enable_if_t<std::disjunction_v<is_property<Lhs>, is_property<Rhs>>, int> = 0>
    bool operator < (const Lhs& lhs, const Rhs& rhs)
    {
        constexpr bool LhsIsProperty = is_property_v<Lhs>;
        constexpr bool RhsIsProperty = is_property_v<Rhs>;
        constexpr bool LhsIsInvalidType = std::is_same_v<std::decay_t<Lhs>, dots::invalid_t>;
        constexpr bool RhsIsInvalidType = std::is_same_v<std::decay_t<Rhs>, dots::invalid_t>;

        if constexpr (LhsIsProperty && RhsIsProperty)
        {
            return rhs.isValid() ? lhs < rhs.storage() : lhs < dots::invalid;
        }
        else if constexpr (LhsIsProperty/* && !RhsIsProperty*/)
        {
            if constexpr (RhsIsInvalidType)
            {
                return false;
            }
            else
            {
                return lhs.isValid() ? lhs.descriptor().valueDescriptor().less(lhs.storage(), rhs) : true;
            }
        }
        else/* if constexpr (!LhsIsProperty && RhsIsProperty) */
        {
            if constexpr (LhsIsInvalidType)
            {
                return rhs.isValid();
            }
            else
            {
                return rhs.isValid() ? rhs.descriptor().valueDescriptor().less(lhs, rhs.storage()) : false;
            }
        }
    }

    template <typename Lhs, typename Rhs, std::enable_if_t<std::disjunction_v<is_property<Lhs>, is_property<Rhs>>, int> = 0>
    bool operator <= (const Lhs& lhs, const Rhs& rhs)
    {
        return !(lhs > rhs);
    }

    template <typename Lhs, typename Rhs, std::enable_if_t<std::disjunction_v<is_property<Lhs>, is_property<Rhs>>, int> = 0>
    bool operator > (const Lhs& lhs, const Rhs& rhs)
    {
        return rhs < lhs;
    }

    template <typename Lhs, typename Rhs, std::enable_if_t<std::disjunction_v<is_property<Lhs>, is_property<Rhs>>, int> = 0>
    bool operator >= (const Lhs& lhs, const Rhs& rhs)
    {
        return !(lhs < rhs);
    }
}
