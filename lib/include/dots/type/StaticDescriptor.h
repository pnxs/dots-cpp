#pragma once
#include <type_traits>
#include <sstream>
#include <iomanip>
#include <charconv>
#include <dots/type/Descriptor.h>
#include <dots/type/DescriptorMap.h>

namespace dots::type
{
    inline DescriptorMap& static_descriptors()
    {
        static DescriptorMap StaticDescriptors;
        return StaticDescriptors;
    }

    struct StaticDescriptor : Descriptor<>
    {
        StaticDescriptor(key_t key, Type type, std::string name, size_t size, size_t alignment) : Descriptor(key, type, std::move(name), size, alignment)
        {
            /* do nothing */
        }

        StaticDescriptor(const StaticDescriptor& other) = delete;
        StaticDescriptor(StaticDescriptor&& other) = delete;
        ~StaticDescriptor() = default;

        StaticDescriptor& operator = (const StaticDescriptor& rhs) = delete;
        StaticDescriptor& operator = (StaticDescriptor&& rhs) = delete;

        Typeless& construct(Typeless& value) const override;
        Typeless& construct(Typeless& value, const Typeless& other) const override;
        Typeless& construct(Typeless& value, Typeless&& other) const override;

        Typeless& constructInPlace(Typeless& value) const override;
        Typeless& constructInPlace(Typeless& value, const Typeless& other) const override;
        Typeless& constructInPlace(Typeless& value, Typeless&& other) const override;

        void destruct(Typeless& value) const override;

        Typeless& assign(Typeless& lhs, const Typeless& rhs) const override;
        Typeless& assign(Typeless& lhs, Typeless&& rhs) const override;

        void swap(Typeless& value, Typeless& other) const override;
        bool equal(const Typeless& lhs, const Typeless& rhs) const override;
        bool less(const Typeless& lhs, const Typeless& rhs) const override;
        using Descriptor<>::dynamicMemoryUsage;

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
        static constexpr T& assign(T& value, Args&&... args)
        {
            static_assert(std::is_constructible_v<T, Args...>, "type is not constructible from passed arguments");
            if constexpr (std::is_constructible_v<T, Args...>)
            {
                value = T(std::forward<Args>(args)...);
            }

            return value;
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

        template <typename T>
        static Descriptor<T>& InitInstance()
        {
            if constexpr (is_dynamic_descriptor_v<Descriptor<T>>)
            {
                throw std::logic_error{ "global descriptor not available because Descriptor<T> dynamic" };
            }
            else
            {
                static auto& Instance = static_descriptors().emplace<Descriptor<T>>();
                return Instance;
            }
        }
    };
}
