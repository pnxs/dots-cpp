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
    using is_property = std::is_base_of<details::property_tag, T>;

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

        template <typename U>
        static constexpr bool is_same_property_v = std::disjunction_v<std::is_same<std::decay_t<U>, Property>, std::is_same<std::decay_t<U>, Derived>>;

        template <typename... Args>
        static constexpr bool is_single_same_property_v = std::conjunction_v<std::bool_constant<sizeof...(Args) == 1>, std::bool_constant<is_same_property_v<Args>>...>;

        template <typename... Args, std::enable_if_t<sizeof...(Args) >= 1 && !is_single_same_property_v<Args...>, int> = 0>
        Property(Args&&... args)
        {
            Property<T, Derived>::construct<false>(std::forward<Args>(args)...);
        }

        template <typename U, std::enable_if_t<std::is_constructible_v<T, std::initializer_list<U>>, int> = 0>
        Property(std::initializer_list<U> init)
        {
            Property<T, Derived>::construct<false>(init);
        }

        template <typename U, std::enable_if_t<!is_same_property_v<U>, int> = 0>
        Derived& operator = (U&& rhs)
        {
            Property<T, Derived>::constructOrAssign(std::forward<U>(rhs));
            return static_cast<Derived&>(*this);
        }

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

        operator T& ()
        {
            return value();
        }

        operator const T& () const
        {
            return value();
        }

        bool isValid() const
        {
            return static_cast<const Derived&>(*this).derivedIsValid();
        }

        template <bool AssertInvalidity = true>
        T& construct(const Property& rhs)
        {
            construct<AssertInvalidity>(rhs.storage());
            return *this;
        }

        template <bool AssertInvalidity = true>
        T& construct(Property&& rhs)
        {
            construct<AssertInvalidity>(std::move(rhs.storage()));
            rhs.destroy();

            return *this;
        }

        template <bool AssertInvalidity = true, typename... Args, std::enable_if_t<!std::disjunction_v<is_property<Args>...>, int> = 0>
        T& construct(Args&&... args)
        {
            if constexpr (AssertInvalidity)
            {
                if (isValid())
                {
                    throw std::runtime_error{ "attempt to construct already valid property: " + descriptor().name() };
                }
            }

            setValid();

            static_assert(!IsTypeless || sizeof...(Args) <= 1, "typeless construct only supports a single argument");
            if constexpr (!IsTypeless)
            {
                Descriptor<T>::constructInPlace(storage(), std::forward<Args>(args)...);
            }
            else if constexpr (sizeof...(Args) == 1)
            {
                descriptor().valueDescriptor().constructInPlace(storage(), std::forward<Args>(args)...);
            }
            else if constexpr (sizeof...(Args) == 0)
            {
                descriptor().valueDescriptor().constructInPlace(storage());
            }

            return storage();
        }

        void destroy()
        {
            if (isValid())
            {
                if constexpr (IsTypeless)
                {
                    descriptor().valueDescriptor().destruct(storage());
                }
                else
                {
                    Descriptor<T>::destruct(storage());
                }

                setInvalid();
            }
        }

        T& value()
        {
            if (!isValid())
            {
                throw std::runtime_error{ "attempt to access invalid property: " + descriptor().name() };
            }

            return storage();
        }

        const T& value() const
        {
            return const_cast<Property&>(*this).value();
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

        template <bool AssertValidity = true>
        T& assign(const Property& rhs)
        {
            assign<AssertValidity>(rhs.storage());
            return *this;
        }

        template <bool AssertValidity = true>
        T& assign(Property&& rhs)
        {
            assign<AssertValidity>(std::move(rhs.storage()));
            rhs.destroy();

            return *this;
        }

        template <bool AssertValidity = true, typename... Args, std::enable_if_t<!std::disjunction_v<is_property<Args>...>, int> = 0>
        T& assign(Args&&... args)
        {
            if constexpr (AssertValidity)
            {
                if (!isValid())
                {
                    throw std::runtime_error{ "attempt to assign invalid property: " + descriptor().name() };
                }
            }

            setValid();

            static_assert(!IsTypeless || sizeof...(Args) <= 1, "typeless assignment only supports a single argument");
            if constexpr (!IsTypeless)
            {
                Descriptor<T>::assign(storage(), std::forward<Args>(args)...);
            }
            else if constexpr (sizeof...(Args) == 1)
            {
                descriptor().valueDescriptor().assign(storage(), std::forward<Args>(args)...);
            }
            else if constexpr (sizeof...(Args) == 0)
            {
                descriptor().valueDescriptor().assign(storage());
            }

            return storage();
        }

        template <typename... Args>
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

        void swap(Property& other)
        {
            if (isValid())
            {
                if (other.isValid())
                {
                    if constexpr (IsTypeless)
                    {
                        return descriptor().valueDescriptor().swap(storage(), other);
                    }
                    else
                    {
                        return Descriptor<T>::swap(storage(), other);
                    }
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

        bool equal(const Property& rhs) const
        {
            return *this == rhs;
        }

        bool less(const T& rhs) const
        {
            return *this < rhs;
        }

        bool less(const Property& rhs) const
        {
            return *this < rhs;
        }

        bool lessEqual(const T& rhs) const
        {
            return *this <= rhs;
        }

        bool lessEqual(const Property& rhs) const
        {
            return *this <= rhs;
        }

        bool greater(const T& rhs) const
        {
            return *this > rhs;
        }

        bool greater(const Property& rhs) const
        {
            return *this > rhs;
        }

        bool greaterEqual(const T& rhs) const
        {
            return *this >= rhs;
        }

        bool greaterEqual(const Property& rhs) const
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

        constexpr T& storage()
        {
            return static_cast<Derived&>(*this).derivedStorage();
        }

        constexpr const T& storage() const
        {
            return const_cast<Property&>(*this).storage();
        }

    protected:

        constexpr Property() = default;
        constexpr Property(const Property& other) = default;
        constexpr Property(Property&& other) = default;
        ~Property() = default;

        constexpr Property& operator = (const Property& rhs) = default;
        constexpr Property& operator = (Property&& rhs) = default;

    private:

        size_t offset() const
        {
            return static_cast<const Derived&>(*this).derivedOffset();
        }

        const PropertySet& validProperties() const
        {
            return static_cast<const Derived&>(*this).derivedValidProperties();
        }

        PropertySet& validProperties()
        {
            return const_cast<PropertySet&>(std::as_const(*this).validProperties());
        }

        void setValid()
        {
            static_cast<Derived&>(*this).derivedSetValid();
        }

        void setInvalid()
        {
            static_cast<Derived&>(*this).derivedSetInvalid();
        }

        size_t derivedOffset() const
        {
            return descriptor().offset();
        }

        const PropertySet& derivedValidProperties() const
        {
            return PropertyArea::GetArea(storage(), offset()).validProperties();
        }

        PropertySet& derivedValidProperties()
        {
            return const_cast<PropertySet&>(std::as_const(*this).derivedValidProperties());
        }

        bool derivedIsValid() const
        {
            return descriptor().set() <= validProperties();
        }

        void derivedSetValid()
        {
            validProperties() += descriptor().set();
        }

        void derivedSetInvalid()
        {
            validProperties() -= descriptor().set();
        }
    };

    template <typename Lhs, typename Rhs, std::enable_if_t<std::disjunction_v<is_property<Lhs>, is_property<Rhs>>, int> = 0>
    bool operator == (const Lhs& lhs, const Rhs& rhs)
    {
        auto equal = [](const auto& lhs, const auto& rhs)
        {
            using property_t = std::decay_t<decltype(lhs)>;

            if (lhs.isValid())
            {
                if constexpr (property_t::IsTypeless)
                {
                    return lhs.descriptor().valueDescriptor().equal(lhs.storage(), rhs);
                }
                else
                {
                    return Descriptor<typename property_t::value_t>::equal(lhs.storage(), rhs);
                }
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
            using property_t = std::decay_t<decltype(lhs)>;

            if (lhs.isValid())
            {
                if constexpr (property_t::IsTypeless)
                {
                    return lhs.descriptor().valueDescriptor().less(lhs.storage(), rhs);
                }
                else
                {
                    return Descriptor<typename property_t::value_t>::less(lhs.storage(), rhs);
                }
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
