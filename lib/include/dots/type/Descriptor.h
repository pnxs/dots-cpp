// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <string>
#include <string_view>
#include <memory>
#include <stdexcept>
#include <dots/tools/shared_ptr_only.h>
#include <dots/type/Typeless.h>

namespace dots::type
{
    enum struct Type : uint8_t
    {
        boolean,
        int8, uint8, int16, uint16, int32, uint32, int64, uint64,
        float32, float64,
        property_set,
        timepoint, steady_timepoint, duration,
        uuid, string,
        Vector,
        Struct, Enum
    };

    template<typename TDescriptor, typename = void>
    struct type_category
    {
        using value_type = void;
    };

    template <typename TDescriptor>
    using type_category_t = typename type_category<TDescriptor>::type;

    template <typename TDescriptor>
    static constexpr Type type_category_v = type_category_t<TDescriptor>::value;

    template<typename TDescriptor, typename = void>
    struct is_category_descriptor : std::conditional_t<std::is_same_v<typename type_category<TDescriptor>::value_type, Type>, std::true_type, std::false_type> {};

    template <typename T>
    using is_category_descriptor_t = typename is_category_descriptor<T>::type;

    template <typename T>
    static constexpr bool is_category_descriptor_v = is_category_descriptor_t<T>::value;

    template <typename = Typeless>
    struct Descriptor;

    template <>
    struct Descriptor<Typeless> : tools::shared_ptr_only, std::enable_shared_from_this<Descriptor<>>
    {
        Descriptor(key_t key, Type type, std::string name, size_t size, size_t alignment);
        Descriptor(const Descriptor& other) = delete;
        Descriptor(Descriptor&& other) = delete;
        ~Descriptor() override = default;

        Descriptor& operator = (const Descriptor& rhs) = delete;
        Descriptor& operator = (Descriptor&& rhs) = delete;

        Type type() const
        {
            return m_type;
        }

        virtual bool isFundamentalType() const;

        const std::string& name() const
        {
            return m_name;
        }

        size_t size() const
        {
            return m_size;
        }

        size_t alignment() const
        {
            return m_alignment;
        }

        virtual Typeless& construct(Typeless& value) const = 0;
        virtual Typeless& construct(Typeless& value, const Typeless& other) const = 0;
        virtual Typeless& construct(Typeless& value, Typeless&& other) const = 0;

        virtual Typeless& constructInPlace(Typeless& value) const = 0;
        virtual Typeless& constructInPlace(Typeless& value, const Typeless& other) const = 0;
        virtual Typeless& constructInPlace(Typeless& value, Typeless&& other) const = 0;

        virtual void destruct(Typeless& value) const = 0;

        virtual Typeless& assign(Typeless& lhs) const = 0;
        virtual Typeless& assign(Typeless& lhs, const Typeless& rhs) const = 0;
        virtual Typeless& assign(Typeless& lhs, Typeless&& rhs) const = 0;
        virtual void swap(Typeless& value, Typeless& other) const = 0;

        virtual bool equal(const Typeless& lhs, const Typeless& rhs) const = 0;
        virtual bool less(const Typeless& lhs, const Typeless& rhs) const = 0;
        bool lessEqual(const Typeless& lhs, const Typeless& rhs) const;
        bool greater(const Typeless& lhs, const Typeless& rhs) const;
        bool greaterEqual(const Typeless& lhs, const Typeless& rhs) const;

        virtual bool usesDynamicMemory() const;
        virtual size_t dynamicMemoryUsage(const Typeless& value) const;

        template <typename T, typename... Args, std::enable_if_t<!std::is_same_v<T, Typeless>, int> = 0>
        static constexpr T& construct(T& value, Args&&... args)
        {
            static_assert(std::is_constructible_v<T, Args...>, "type is not constructible from passed arguments");
            if constexpr (std::is_constructible_v<T, Args...>)
            {
                ::new(static_cast<void*>(::std::addressof(value))) T(std::forward<Args>(args)...);
            }

            return value;
        }

        template <typename T, typename... Args, std::enable_if_t<!std::is_same_v<T, Typeless>, int> = 0>
        static constexpr T& constructInPlace(T& value, Args&&... args)
        {
            return construct(value, std::forward<Args>(args)...);
        }

        template <typename T, std::enable_if_t<!std::is_same_v<T, Typeless>, int> = 0>
        static constexpr void destruct(T& value)
        {
            value.~T();
        }

        template <typename T, typename... Args, std::enable_if_t<!std::is_same_v<T, Typeless>, int> = 0>
        static constexpr T& assign(T& lhs, Args&&... args)
        {
            static_assert(std::is_constructible_v<T, Args...>, "type is not constructible from passed arguments");
            if constexpr (std::is_constructible_v<T, Args...>)
            {
                lhs = T(std::forward<Args>(args)...);
            }

            return lhs;
        }

        template <typename T, std::enable_if_t<!std::is_same_v<T, Typeless>, int> = 0>
        static constexpr void swap(T& lhs, T& rhs)
        {
            std::swap(lhs, rhs);
        }

        template <typename T, typename U, std::enable_if_t<!std::is_same_v<T, Typeless>, int> = 0>
        static constexpr bool equal(const T& lhs, const U& rhs)
        {
            return std::equal_to<T>{}(lhs, rhs);
        }

        template <typename T, typename U, std::enable_if_t<!std::is_same_v<T, Typeless>, int> = 0>
        static constexpr bool less(const T& lhs, const U& rhs)
        {
            return std::less<T>{}(lhs, rhs);
        }

        template <typename T, typename U, std::enable_if_t<!std::is_same_v<T, Typeless>, int> = 0>
        static constexpr bool lessEqual(const T& lhs, const U& rhs)
        {
            return !greater(lhs, rhs);
        }

        template <typename T, typename U, std::enable_if_t<!std::is_same_v<T, Typeless>, int> = 0>
        static constexpr bool greater(const T& lhs, const U& rhs)
        {
            return less<T>(rhs, lhs);
        }

        template <typename T, typename U, std::enable_if_t<!std::is_same_v<T, Typeless>, int> = 0>
        static constexpr bool greaterEqual(const T& lhs, const U& rhs)
        {
            return !less(lhs, rhs);
        }

        template <typename T, std::enable_if_t<!std::is_same_v<T, Typeless>, int> = 0>
        size_t dynamicMemoryUsage(const T& value) const
        {
            return dynamicMemoryUsage(Typeless::From(value));
        }

        template <typename TDescriptor>
        constexpr bool is() const
        {
            constexpr bool IsDescriptor = std::is_base_of_v<Descriptor<>, TDescriptor>;
            static_assert(IsDescriptor, "TDescriptor has to be a descriptor");

            if constexpr (IsDescriptor)
            {
                if constexpr (std::is_same_v<TDescriptor, Descriptor<>>)
                {
                    return true;
                }
                else if constexpr (is_category_descriptor_v<TDescriptor>)
                {
                    return type() == type_category_v<TDescriptor>;
                }
                else
                {
                    return typeid(*this) == typeid(TDescriptor);
                }
            }
            else
            {
                return false;
            }
        }

        template <typename... TDescriptors>
        constexpr bool isAny() const
        {
            return (is<TDescriptors>() || ...);
        }

        template <typename TDescriptor>
        void assertIs() const
        {
            if (!is<TDescriptor>())
            {
                throw std::logic_error{ "descriptor '" + name() + "' does not have expected type" };
            }
        }

        template <typename... TDescriptors>
        void assertIsAny() const
        {
            if (!isAny<TDescriptors...>())
            {
                throw std::logic_error{ "descriptor '" + name() + "' does not have any of expected types" };
            }
        }

        template <typename TDescriptor>
        constexpr const TDescriptor* as() const
        {
            constexpr bool IsDescriptor = std::is_base_of_v<Descriptor<>, TDescriptor>;
            static_assert(IsDescriptor, "TDescriptor has to be a descriptor");

            if constexpr (IsDescriptor)
            {
                if (is<TDescriptor>())
                {
                    return static_cast<const TDescriptor*>(this);
                }
                else
                {
                    return nullptr;
                }
            }
            else
            {
                return nullptr;
            }
        }

        template <typename TDescriptor>
        constexpr TDescriptor* as()
        {
            return const_cast<TDescriptor*>(std::as_const(*this).as<TDescriptor>());
        }

        template <typename TDescriptor, bool Safe = false>
        constexpr const TDescriptor& to() const
        {
            constexpr bool IsDescriptor = std::is_base_of_v<Descriptor<>, TDescriptor>;
            static_assert(IsDescriptor, "TDescriptor has to be a descriptor");

            if constexpr (IsDescriptor)
            {
                if constexpr (Safe)
                {
                    if (!is<TDescriptor>())
                    {
                        throw std::logic_error{ std::string{ "type mismatch in safe descriptor conversion" } };
                    }
                }

                return static_cast<const TDescriptor&>(*this);
            }
            else
            {
                return std::declval<const TDescriptor>();
            }
        }

        template <typename TDescriptor, bool Safe = false>
        constexpr TDescriptor& to()
        {
            return const_cast<TDescriptor&>(std::as_const(*this).to<TDescriptor, Safe>());
        }

        static bool IsFundamentalType(const Descriptor& descriptor);

    private:

        static bool IsFundamentalType(Type type);

        Type m_type;
        std::string m_name;
        size_t m_size;
        size_t m_alignment;
    };

    template <typename TDescriptor, typename... Args>
    std::shared_ptr<TDescriptor> make_descriptor(Args&&... args)
    {
        static_assert(std::is_base_of_v<Descriptor<>, TDescriptor>, "TDescriptor must be derived from Descriptor<>");
        static_assert(std::is_constructible_v<TDescriptor, tools::shared_ptr_only::key_t, Args...>, "TDescriptor is not constructible from Args");

        return tools::make_shared_ptr_only<TDescriptor>(std::forward<Args>(args)...);
    }

    template<typename T, typename = void>
    constexpr bool is_defined_v = false;

    template<typename T>
    constexpr bool is_defined_v<T, decltype(sizeof(T), void())> = true;

    template<typename T>
    using is_defined_t = std::conditional_t<is_defined_v<T>, std::true_type, std::false_type>;

    template <typename T>
    struct is_descriptor : std::false_type {};

    template <typename T>
    struct is_descriptor<Descriptor<T>> : std::true_type {};

    template <typename T>
    using is_descriptor_t = typename is_descriptor<T>::type;

    template <typename T>
    constexpr bool is_descriptor_v = is_descriptor_t<T>::value;

    template<typename T, typename = void>
    struct is_dynamic_descriptor: std::false_type {};

    template<typename T>
    struct is_dynamic_descriptor<T, std::void_t<decltype(T::IsDynamic)>> : std::integral_constant<bool, T::IsDynamic> {};

    template <typename T>
    using is_dynamic_descriptor_t = typename is_dynamic_descriptor<T>::type;

    template <typename T>
    static constexpr bool is_dynamic_descriptor_v = is_dynamic_descriptor_t<T>::value;

    template <typename T>
    struct described_type
    {
        static_assert(is_descriptor_v<T>, "T has to be a descriptor");
    };

    template <typename T>
    struct described_type<Descriptor<T>>
    {
        static_assert(is_descriptor_v<Descriptor<T>>, "T has to be a descriptor");
        using type = T;
    };

    template <typename T>
    using described_type_t = typename described_type<T>::type;

    template <typename T>
    using has_descriptor_t = std::conditional_t<is_defined_v<Descriptor<T>>, std::true_type, std::false_type>;

    template <typename T>
    constexpr bool has_descriptor_v = has_descriptor_t<T>::value;
}
