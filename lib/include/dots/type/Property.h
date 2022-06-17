#pragma once
#include <type_traits>
#include <cstddef>
#include <dots/type/PropertyArea.h>
#include <dots/type/PropertyDescriptor.h>

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

        template <typename... Args, std::enable_if_t<sizeof...(Args) >= 1, int> = 0>
        T& operator () (Args&&... args)
        {
            return construct(std::forward<Args>(args)...);
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

        template <bool AssertInvalidity = true, typename D>
        Derived& construct(const Property<T, D>& rhs)
        {
            assertNotIsValid<AssertInvalidity>();

            if (rhs.isValid())
            {
                construct<false>(rhs.storage());
            }

            return static_cast<Derived&>(*this);
        }

        template <bool AssertInvalidity = true, typename D>
        Derived& construct(Property<T, D>&& rhs)
        {
            assertNotIsValid<AssertInvalidity>();

            if (rhs.isValid())
            {
                construct<false>(std::move(rhs.storage()));
                rhs.destroy();
            }

            return static_cast<Derived&>(*this);
        }

        template <bool AssertInvalidity = true, typename... Args, std::enable_if_t<!std::disjunction_v<is_property<Args>...>, int> = 0>
        T& construct(Args&&... args)
        {
            assertNotIsValid<AssertInvalidity>();
            setValid();

            static_assert(!IsTypeless || sizeof...(Args) <= 1, "typeless construct only supports a single argument");
            descriptor().valueDescriptor().constructInPlace(storage(), std::forward<Args>(args)...);

            return storage();
        }

        void destroy()
        {
            if (isValid())
            {
                descriptor().valueDescriptor().destruct(storage());
                setInvalid();
            }
        }

        const T& value() const
        {
            assertIsValid<true>();
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
        T& constructOrValue(Args&&... args)
        {
            if (isValid())
            {
                return storage();
            }
            else
            {
                return construct<false>(std::forward<Args>(args)...);
            }
        }

        template <bool AssertValidity = true, typename D>
        Derived& assign(const Property<T, D>& rhs)
        {
            assertIsValid<AssertValidity>();

            if (rhs.isValid())
            {
                assign<false>(rhs.storage());
            }
            else
            {
                destroy();
            }

            return static_cast<Derived&>(*this);
        }

        template <bool AssertValidity = true, typename D>
        Derived& assign(Property<T, D>&& rhs)
        {
            assertIsValid<AssertValidity>();

            if (rhs.isValid())
            {
                assign<false>(std::move(rhs.storage()));
                rhs.destroy();
            }
            else
            {
                destroy();
            }

            return static_cast<Derived&>(*this);
        }

        template <bool AssertValidity = true, typename... Args, std::enable_if_t<!std::disjunction_v<is_property<Args>...>, int> = 0>
        T& assign(Args&&... args)
        {
            assertIsValid<AssertValidity>();
            setValid();

            static_assert(!IsTypeless || sizeof...(Args) <= 1, "typeless assignment only supports a single argument");
            descriptor().valueDescriptor().assign(storage(), std::forward<Args>(args)...);

            return storage();
        }

        template <typename D>
        Derived& constructOrAssign(const Property<T, D>& rhs)
        {
            if (isValid())
            {
                assign<false>(rhs);
            }
            else
            {
                construct<false>(rhs);
            }

            return static_cast<Derived&>(*this);
        }

        template <typename D>
        Derived& constructOrAssign(Property<T, D>&& rhs)
        {
            if (isValid())
            {
                assign<false>(std::move(rhs));
            }
            else
            {
                construct<false>(std::move(rhs));
            }

            return static_cast<Derived&>(*this);
        }

        template <typename... Args, std::enable_if_t<!std::disjunction_v<is_property<Args>...>, int> = 0>
        T& constructOrAssign(Args&&... args)
        {
            if (isValid())
            {
                return assign<false>(std::forward<Args>(args)...);
            }
            else
            {
                return construct<false>(std::forward<Args>(args)...);
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
                    other.template construct<false>(std::move(storage()));
                    destroy();
                }
            }
            else if (other.isValid())
            {
                construct<false>(std::move(other.storage()));
                other.destroy();
            }
        }

        bool equal(const T& rhs) const
        {
            return *this == rhs;
        }

        template <typename D>
        bool equal(const Property<T, D>& rhs) const
        {
            return *this == rhs;
        }

        bool less(const T& rhs) const
        {
            return *this < rhs;
        }

        template <typename D>
        bool less(const Property<T, D>& rhs) const
        {
            return *this < rhs;
        }

        bool lessEqual(const T& rhs) const
        {
            return *this <= rhs;
        }

        template <typename D>
        bool lessEqual(const Property<T, D>& rhs) const
        {
            return *this <= rhs;
        }

        bool greater(const T& rhs) const
        {
            return *this > rhs;
        }

        template <typename D>
        bool greater(const Property<T, D>& rhs) const
        {
            return *this > rhs;
        }

        bool greaterEqual(const T& rhs) const
        {
            return *this >= rhs;
        }

        template <typename D>
        bool greaterEqual(const Property<T, D>& rhs) const
        {
            return *this >= rhs;
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

    private:

        template <bool AssertValidity>
        void assertIsValid() const
        {
            if constexpr (AssertValidity)
            {
                if (!isValid())
                {
                    throw std::runtime_error{ "property is expected to be valid but it is not: " + descriptor().name() };
                }
            }
        }

        template <bool AssertInvalidity>
        void assertNotIsValid() const
        {
            if constexpr (AssertInvalidity)
            {
                if (isValid())
                {
                    throw std::runtime_error{ "property is expected to be invalid but it is not: " + descriptor().name() };
                }
            }
        }

        void setValid()
        {
            static_cast<Derived&>(*this).derivedSetValid();
        }

        void setInvalid()
        {
            static_cast<Derived&>(*this).derivedSetInvalid();
        }
    };

    template <typename Lhs, typename Rhs, std::enable_if_t<std::disjunction_v<is_property<Lhs>, is_property<Rhs>>, int> = 0>
    bool operator == (const Lhs& lhs, const Rhs& rhs)
    {
        auto equal = [](const auto& lhs, const auto& rhs)
        {
            if (lhs.isValid())
            {
                return lhs.descriptor().valueDescriptor().equal(lhs.storage(), rhs);
            }
            else
            {
                return false;
            }
        };

        if constexpr (is_property_v<Lhs>)
        {
            if constexpr (is_property_v<Rhs>)
            {
                if (rhs.isValid())
                {
                    return equal(lhs, rhs.storage());
                }
                else
                {
                    return !lhs.isValid();
                }
            }
            else
            {
                return equal(lhs, rhs);
            }
        }
        else
        {
            return equal(rhs, lhs);
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
        auto less = [](const auto& lhs, const auto& rhs)
        {
            if (lhs.isValid())
            {
                return lhs.descriptor().valueDescriptor().less(lhs.storage(), rhs);
            }
            else
            {
                return false;
            }
        };

        if constexpr (is_property_v<Lhs>)
        {
            if constexpr (is_property_v<Rhs>)
            {
                if (rhs.isValid())
                {
                    return less(lhs, rhs.storage());
                }
                else
                {
                    return lhs.isValid();
                }
            }
            else
            {
                return less(lhs, rhs);
            }
        }
        else
        {
            return less(rhs, lhs);
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
