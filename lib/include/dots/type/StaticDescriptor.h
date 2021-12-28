#pragma once
#include <type_traits>
#include <string_view>
#include <sstream>
#include <iomanip>
#include <charconv>
#include <dots/type/Descriptor.h>
#include <dots/type/DescriptorMap.h>

namespace dots::type
{
    namespace details
    {
        template<typename U, typename = void>
        struct use_static_descriptor_operations: std::true_type {};
        template<typename U>
        struct use_static_descriptor_operations<U, std::void_t<decltype(U::_UseStaticDescriptorOperations)>> : std::integral_constant<bool, U::_UseStaticDescriptorOperations> {};
        template <typename U>
        using use_static_descriptor_operations_t = typename use_static_descriptor_operations<U>::type;
        template <typename U>
        static constexpr bool use_static_descriptor_operations_v = use_static_descriptor_operations_t<U>::value;
    }

    inline DescriptorMap& static_descriptors()
    {
        static DescriptorMap StaticDescriptors;
        return StaticDescriptors;
    }

    template <typename T, typename Base = Descriptor<Typeless>, bool UseStaticDescriptorOperations = details::use_static_descriptor_operations_v<T>, typename = void>
    struct StaticDescriptor;

    template <typename T, typename Base>
    struct StaticDescriptor<T, Base, false> : Base
    {
        using key_t = typename Base::key_t;

        template <typename Base_ = Base, std::enable_if_t<std::is_same_v<Base_, Descriptor<Typeless>>, int> = 0>
        StaticDescriptor(key_t key, Type type, std::string name) : Base(key, type, std::move(name), sizeof(T), alignof(T))
        {
            /* do nothing */
        }

        template <typename Base_ = Base, typename... Args, std::enable_if_t<!std::is_same_v<Base_, Descriptor<Typeless>>, int> = 0>
        constexpr StaticDescriptor(Args&&... args) : Base(std::forward<Args>(args)...)
        {
            /* do nothing */
        }

        StaticDescriptor(const StaticDescriptor& other) = delete;
        StaticDescriptor(StaticDescriptor&& other) = delete;
        ~StaticDescriptor() = default;

        StaticDescriptor& operator = (const StaticDescriptor& rhs) = delete;
        StaticDescriptor& operator = (StaticDescriptor&& rhs) = delete;

        using Base::construct;
        using Base::constructInPlace;
        using Base::destruct;
        using Base::assign;
        using Base::swap;
        using Base::equal;
        using Base::less;
        using Base::lessEqual;
        using Base::greater;
        using Base::greaterEqual;
        using Base::dynamicMemoryUsage;

        template <typename... Args>
        static constexpr T& construct(T& value, Args&&... args)
        {
            static_assert(std::is_constructible_v<T, Args...>, "type is not constructible from passed arguments");
            if constexpr (std::is_constructible_v<T, Args...>)
            {
                ::new(static_cast<void*>(::std::addressof(value))) T(std::forward<Args>(args)...);
            }

            return value;
        }

        template <typename... Args>
        static constexpr T& constructInPlace(T& value, Args&&... args)
        {
            return construct(value, std::forward<Args>(args)...);
        }

        static constexpr void destruct(T& value)
        {
            value.~T();
        }

        template <typename... Args>
        static constexpr T& assign(T& value, Args&&... args)
        {
            static_assert(std::is_constructible_v<T, Args...>, "type is not constructible from passed arguments");
            if constexpr (std::is_constructible_v<T, Args...>)
            {
                value = T(std::forward<Args>(args)...);
            }

            return value;
        }

        static constexpr void swap(T& lhs, T& rhs)
        {
            std::swap(lhs, rhs);
        }

        static constexpr bool equal(const T& lhs, const T& rhs)
        {
            return std::equal_to<T>{}(lhs, rhs);
        }

        static constexpr bool less(const T& lhs, const T& rhs)
        {
            return std::less<T>{}(lhs, rhs);
        }

        static constexpr bool lessEqual(const T& lhs, const T& rhs)
        {
            return !greater(lhs, rhs);
        }

        static constexpr bool greater(const T& lhs, const T& rhs)
        {
            return less(rhs, lhs);
        }

        static constexpr bool greaterEqual(const T& lhs, const T& rhs)
        {
            return !less(lhs, rhs);
        }

        size_t dynamicMemoryUsage(const T& value) const
        {
            return Base::dynamicMemoryUsage(Typeless::From(value));
        }

        std::shared_ptr<const Descriptor<T>> shared_from_this() const
        {
            return std::static_pointer_cast<const Descriptor<T>>(std::enable_shared_from_this<Descriptor<>>::shared_from_this());
        }

        std::shared_ptr<Descriptor<T>> shared_from_this()
        {
            return std::static_pointer_cast<Descriptor<T>>(std::enable_shared_from_this<Descriptor<>>::shared_from_this());
        }

        std::weak_ptr<const Descriptor<T>> weak_from_this() const noexcept
        {
            return std::enable_shared_from_this<Descriptor<>>::weak_from_this();
        }

        std::weak_ptr<Descriptor<T>> weak_from_this() noexcept
        {
            return std::enable_shared_from_this<Descriptor<>>::weak_from_this();
        }

        static Descriptor<T>& InitInstance()
        {
            return *InitInstancePtr();
        }

        static Descriptor<T>& Instance()
        {
            return *M_instanceRawPtr;
        }

    private:

        template <bool AssertNotDynamic = true>
        static std::shared_ptr<Descriptor<T>>& InitInstancePtr()
        {
            if constexpr (is_dynamic_descriptor_v<Descriptor<T>>)
            {
                if constexpr (AssertNotDynamic)
                {
                    throw std::logic_error{ "global descriptor not available because Descriptor<T> dynamic" };
                }
                else
                {
                    return M_instanceStorage;
                }
            }
            else
            {
                if (M_instanceStorage == nullptr)
                {
                    M_instanceStorage = static_descriptors().emplace<Descriptor<T>>().shared_from_this();
                }

                return M_instanceStorage;
            }
        }

        inline static std::shared_ptr<Descriptor<T>> M_instanceStorage;
        inline static Descriptor<T>* M_instanceRawPtr = InitInstancePtr<false>().get();
    };

    template <typename T, typename Base>
    struct StaticDescriptor<T, Base, true> : StaticDescriptor<T, Base, false>
    {
        template <typename... Args>
        StaticDescriptor(Args&&... args) : StaticDescriptor<T, Base, false>(std::forward<Args>(args)...)
        {
            /* do nothing */
        }

        StaticDescriptor(const StaticDescriptor& other) = delete;
        StaticDescriptor(StaticDescriptor&& other) = delete;
        ~StaticDescriptor() = default;

        StaticDescriptor& operator = (const StaticDescriptor& rhs) = delete;
        StaticDescriptor& operator = (StaticDescriptor&& rhs) = delete;

        using StaticDescriptor<T, Base, false>::construct;
        using StaticDescriptor<T, Base, false>::constructInPlace;
        using StaticDescriptor<T, Base, false>::destruct;
        using StaticDescriptor<T, Base, false>::assign;
        using StaticDescriptor<T, Base, false>::swap;
        using StaticDescriptor<T, Base, false>::equal;
        using StaticDescriptor<T, Base, false>::less;
        using StaticDescriptor<T, Base, false>::lessEqual;
        using StaticDescriptor<T, Base, false>::greater;
        using StaticDescriptor<T, Base, false>::greaterEqual;
        using StaticDescriptor<T, Base, false>::dynamicMemoryUsage;

        Typeless& construct(Typeless& value) const override
        {
            if constexpr (std::is_default_constructible_v<T>)
            {
                return reinterpret_cast<Typeless&>(construct(reinterpret_cast<T&>(value)));
            }
            else
            {
                throw std::logic_error{ "construct has to be overridden in sub-class because T is not default constructible" };
            }
        }

        Typeless& construct(Typeless& value, const Typeless& other) const override
        {
            return reinterpret_cast<Typeless&>(construct(reinterpret_cast<T&>(value), reinterpret_cast<const T&>(other)));
        }

        Typeless& construct(Typeless& value, Typeless&& other) const override
        {
            return reinterpret_cast<Typeless&>(construct(reinterpret_cast<T&>(value), reinterpret_cast<T&&>(other)));
        }

        Typeless& constructInPlace(Typeless& value) const override
        {
            return construct(value);
        }

        Typeless& constructInPlace(Typeless& value, const Typeless& other) const override
        {
            return construct(value, other);
        }

        Typeless& constructInPlace(Typeless& value, Typeless&& other) const override
        {
            return construct(value, std::move(other));
        }

        void destruct(Typeless& value) const override
        {
            destruct(reinterpret_cast<T&>(value));
        }

        Typeless& assign(Typeless& lhs, const Typeless& rhs) const override
        {
            return reinterpret_cast<Typeless&>(assign(reinterpret_cast<T&>(lhs), reinterpret_cast<const T&>(rhs)));
        }

        Typeless& assign(Typeless& lhs, Typeless&& rhs) const override
        {
            return reinterpret_cast<Typeless&>(assign(reinterpret_cast<T&>(lhs), reinterpret_cast<T&&>(rhs)));
        }

        void swap(Typeless& value, Typeless& other) const override
        {
            swap(reinterpret_cast<T&>(value), reinterpret_cast<T&>(other));
        }

        bool equal(const Typeless& lhs, const Typeless& rhs) const override
        {
            return equal(reinterpret_cast<const T&>(lhs), reinterpret_cast<const T&>(rhs));
        }

        bool less(const Typeless& lhs, const Typeless& rhs) const override
        {
            return less(reinterpret_cast<const T&>(lhs), reinterpret_cast<const T&>(rhs));
        }

        bool lessEqual(const Typeless& lhs, const Typeless& rhs) const override
        {
            return lessEqual(reinterpret_cast<const T&>(lhs), reinterpret_cast<const T&>(rhs));
        }

        bool greater(const Typeless& lhs, const Typeless& rhs) const override
        {
            return greater(reinterpret_cast<const T&>(lhs), reinterpret_cast<const T&>(rhs));
        }

        bool greaterEqual(const Typeless& lhs, const Typeless& rhs) const override
        {
            return greaterEqual(reinterpret_cast<const T&>(lhs), reinterpret_cast<const T&>(rhs));
        }
    };
}