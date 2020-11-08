#pragma once
#include <type_traits>
#include <string_view>
#include <sstream>
#include <iomanip>
#include <map>
#include <charconv>
#include <dots/type/Descriptor.h>

namespace dots::type
{
    struct StaticDescriptorMap
    {
        static std::shared_ptr<Descriptor<>> Emplace(std::shared_ptr<Descriptor<>> descriptor);

        static std::shared_ptr<Descriptor<>> Find(const std::string_view& name)
        {
            auto it = DescriptorsMutable().find(name);
            return it == DescriptorsMutable().end() ? nullptr : it->second;
        }

        static const std::map<std::string_view, std::shared_ptr<Descriptor<>>>& Descriptors()
        {
            return DescriptorsMutable();
        }

    private:

        static std::map<std::string_view, std::shared_ptr<Descriptor<>>>& DescriptorsMutable()
        {
            static std::map<std::string_view, std::shared_ptr<Descriptor<>>> Descriptors_;
            return Descriptors_;
        }
    };

    template <typename T, typename Base = Descriptor<Typeless>>
    struct StaticDescriptor : Base
    {
        template <typename Base_ = Base, std::enable_if_t<std::is_same_v<Base_, Descriptor<Typeless>>, int> = 0>
        StaticDescriptor(Type type, std::string name) : Base(type, std::move(name), sizeof(T), alignof(T))
        {
            /* do nothing */
        }

        template <typename Base_ = Base, typename... Args, std::enable_if_t<!std::is_same_v<Base_, Descriptor<Typeless>>, int> = 0>
        constexpr StaticDescriptor(Args&&... args) : Base(std::forward<Args>(args)...)
        {
            /* do nothing */
        }

        StaticDescriptor(const StaticDescriptor& other) = default;
        StaticDescriptor(StaticDescriptor&& other) = default;
        ~StaticDescriptor() = default;

        StaticDescriptor& operator = (const StaticDescriptor& rhs) = default;
        StaticDescriptor& operator = (StaticDescriptor&& rhs) = default;

        using Base::assign;
        using Base::swap;
        using Base::equal;
        using Base::less;
        using Base::lessEqual;
        using Base::greater;
        using Base::greaterEqual;

        Typeless& construct(Typeless& value) const override
        {
            if constexpr (UseStaticDescriptorOperations)
            {
                if constexpr (std::is_default_constructible_v<T>)
                {
                    return reinterpret_cast<Typeless&>(StaticDescriptor::construct(reinterpret_cast<T&>(value)));
                }
                else
                {
                    throw std::logic_error{ "construct has to be overridden in sub-class because T is not default constructible" };
                }
            }
            else
            {
                return Base::construct(value);
            }
        }

        Typeless& construct(Typeless& value, const Typeless& other) const override
        {
            if constexpr (UseStaticDescriptorOperations)
            {
                return reinterpret_cast<Typeless&>(StaticDescriptor::construct(reinterpret_cast<T&>(value), reinterpret_cast<const T&>(other)));
            }
            else
            {
                return Base::construct(value, other);
            }
        }

        Typeless& construct(Typeless& value, Typeless&& other) const override
        {
            if constexpr (UseStaticDescriptorOperations)
            {
                return reinterpret_cast<Typeless&>(StaticDescriptor::construct(reinterpret_cast<T&>(value), reinterpret_cast<T&&>(other)));
            }
            else
            {
                return Base::construct(value, std::move(other));
            }
        }

        void destruct(Typeless& value) const override
        {
            if constexpr (UseStaticDescriptorOperations)
            {
                StaticDescriptor::destruct(reinterpret_cast<T&>(value));
            }
            else
            {
                Base::destruct(value);
            }
        }

        Typeless& assign(Typeless& lhs, const Typeless& rhs) const override
        {
            if constexpr (UseStaticDescriptorOperations)
            {
                return reinterpret_cast<Typeless&>(StaticDescriptor::assign(reinterpret_cast<T&>(lhs), reinterpret_cast<const T&>(rhs)));
            }
            else
            {
                return Base::assign(lhs, rhs);
            }
        }

        Typeless& assign(Typeless& lhs, Typeless&& rhs) const override
        {
            if constexpr (UseStaticDescriptorOperations)
            {
                return reinterpret_cast<Typeless&>(StaticDescriptor::assign(reinterpret_cast<T&>(lhs), reinterpret_cast<T&&>(rhs)));
            }
            else
            {
                return Base::assign(lhs, std::move(rhs));
            }
        }

        void swap(Typeless& value, Typeless& other) const override
        {
            if constexpr (UseStaticDescriptorOperations)
            {
                StaticDescriptor::swap(reinterpret_cast<T&>(value), reinterpret_cast<T&>(other));
            }
            else
            {
                Base::swap(value, other);
            }
        }

        bool equal(const Typeless& lhs, const Typeless& rhs) const override
        {
            if constexpr (UseStaticDescriptorOperations)
            {
                return StaticDescriptor::equal(reinterpret_cast<const T&>(lhs), reinterpret_cast<const T&>(rhs));
            }
            else
            {
                return Base::equal(lhs, rhs);
            }
        }

        bool less(const Typeless& lhs, const Typeless& rhs) const override
        {
            if constexpr (UseStaticDescriptorOperations)
            {
                return StaticDescriptor::less(reinterpret_cast<const T&>(lhs), reinterpret_cast<const T&>(rhs));
            }
            else
            {
                return Base::less(lhs, rhs);
            }
        }

        bool lessEqual(const Typeless& lhs, const Typeless& rhs) const override
        {
            if constexpr (UseStaticDescriptorOperations)
            {
                return StaticDescriptor::lessEqual(reinterpret_cast<const T&>(lhs), reinterpret_cast<const T&>(rhs));
            }
            else
            {
                return Base::lessEqual(lhs, rhs);
            }
        }

        bool greater(const Typeless& lhs, const Typeless& rhs) const override
        {
            if constexpr (UseStaticDescriptorOperations)
            {
                return StaticDescriptor::greater(reinterpret_cast<const T&>(lhs), reinterpret_cast<const T&>(rhs));
            }
            else
            {
                return Base::greater(lhs, rhs);
            }
        }

        bool greaterEqual(const Typeless& lhs, const Typeless& rhs) const override
        {
            if constexpr (UseStaticDescriptorOperations)
            {
                return StaticDescriptor::greaterEqual(reinterpret_cast<const T&>(lhs), reinterpret_cast<const T&>(rhs));
            }
            else
            {
                return Base::greaterEqual(lhs, rhs);
            }
        }

        size_t dynamicMemoryUsage(const Typeless& value) const override
        {
            if constexpr (UseStaticDescriptorOperations)
            {
                return StaticDescriptor::dynamicMemoryUsage(reinterpret_cast<const T&>(value));
            }
            else
            {
                return Base::dynamicMemoryUsage(value);
            }
        }

        bool usesDynamicMemory() const override
        {
            if constexpr (UseStaticDescriptorOperations)
            {
                return false;
            }
            else
            {
                return Base::usesDynamicMemory();
            }
        }

        void fromString(Typeless& storage, const std::string_view& value) const override
        {
            fromString(storage.to<T>(), value);
        }

        std::string toString(const Typeless& value) const override
        {
            return toString(value.to<T>());
        }

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

        static constexpr size_t dynamicMemoryUsage(const T&/* value*/)
        {
            return 0;
        }

        void fromString(T& storage, const std::string_view& value) const
        {
            // TODO: use std::from_chars where applicable

            if constexpr (std::is_same_v<T, bool>)
            {
                if (value == "1" || value == "true")
                {
                    construct(storage, true);
                }
                else if (value == "0" || value == "false")
                {
                    construct(storage, false);
                }
                else
                {
                    throw std::runtime_error{ "cannot construct boolean from string: " + std::string{ value } };
                }

            }
            else if constexpr (std::is_constructible_v<T, std::string_view>)
            {
                construct(storage, value);
            }
            else if constexpr (std::is_integral_v<T>)
            {
                T t;

                if (std::from_chars_result result = std::from_chars(value.data(), value.data() + value.size(), t); result.ec != std::errc{})
                {
                    throw std::runtime_error{ "could not construct integer from string: " + std::string{ value } + " -> " + std::make_error_code(result.ec).message() };
                }

                construct(storage, t);
            }
            else if constexpr (std::disjunction_v<std::is_floating_point<T>, is_istreamable<T>>)
            {
                T t;
                std::istringstream iss{ value.data() };
                iss.exceptions(std::istringstream::failbit | std::istringstream::badbit);
                iss >> t;
                construct(storage, std::move(t));
            }
            else
            {
                return Descriptor<>::fromString(Typeless::From(storage), value);
            }
        }

        std::string toString(const T& value) const
        {
            // TODO: use std::to_chars where applicable

            if constexpr (std::is_same_v<T, bool>)
            {
                return value ? "true" : "false";
            }
            else if constexpr (std::is_integral_v<T>)
            {
                char buffer[128];

                if (auto [last, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value); ec == std::errc{})
                {
                    return std::string{ buffer, last };
                }
                else
                {
                    throw std::runtime_error{ "could not convert value to string: -> " + std::make_error_code(ec).message() };
                }
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                std::ostringstream oss;
                oss.exceptions(std::istringstream::failbit | std::istringstream::badbit);
                oss << std::setprecision(std::numeric_limits<T>::digits10 + 1) << value;

                return oss.str();
            }
            else if constexpr (std::is_constructible_v<std::string, T>)
            {
                return value;
            }
            else if constexpr (is_ostreamable_v<T>)
            {
                std::ostringstream oss;
                oss.exceptions(std::ostringstream::failbit | std::ostringstream::badbit);
                oss << value;

                return oss.str();
            }
            else
            {
                return Descriptor<>::toString(Typeless::From(value));
            }
        }

        static const std::shared_ptr<Descriptor<T>>& InstancePtr()
        {
            if constexpr (std::is_default_constructible_v<Descriptor<T>>)
            {
                if constexpr (std::is_default_constructible_v<Descriptor<T>>)
                {
                    if (M_descriptor == nullptr)
                    {
                        M_descriptor = std::static_pointer_cast<Descriptor<T>>(StaticDescriptorMap::Emplace(std::make_shared<Descriptor<T>>()));
                    }

                    return M_descriptor;
                }
                else
                {
                    return nullptr;
                }
            }
            else
            {
                throw std::logic_error{ "global descriptor not available because Descriptor<T> is not default constructible" };
            }
        }

        static const Descriptor<T>& Instance()
        {
            return *InstancePtr();
        }

    protected:

        template<typename U, typename = void>
        struct use_static_descriptor_operations: std::true_type {};
        template<typename U>
        struct use_static_descriptor_operations<U, std::void_t<decltype(U::_UseStaticDescriptorOperations)>> : std::integral_constant<bool, U::_UseStaticDescriptorOperations> {};
        template <typename U>
        using use_static_descriptor_operations_t = typename use_static_descriptor_operations<U>::type;
        template <typename U>
        static constexpr bool use_static_descriptor_operations_v = use_static_descriptor_operations_t<U>::value;

        static constexpr bool UseStaticDescriptorOperations = use_static_descriptor_operations_v<T>;

    private:

        template<typename U, typename = void>
        struct is_ostreamable: std::false_type {};
        template<typename U>
        struct is_ostreamable<U, std::void_t<decltype(std::declval<std::ostream&>()<<std::declval<const U&>())>> : std::true_type {};
        template <typename U>
        using is_ostreamable_t = typename is_ostreamable<U>::type;
        template <typename U>
        static constexpr bool is_ostreamable_v = is_ostreamable_t<U>::value;

        template<typename U, typename = void>
        struct is_istreamable: std::false_type {};
        template<typename U>
        struct is_istreamable<U, std::void_t<decltype(std::declval<std::istream&>()>>std::declval<U&>())>> : std::true_type {};
        template <typename U>
        using is_istreamable_t = typename is_istreamable<U>::type;
        template <typename U>
        static constexpr bool is_istreamable_v = is_istreamable_t<U>::value;

        inline static std::shared_ptr<Descriptor<T>> M_descriptor;

        // initialize a reference to the descriptor to ensure that it is always added to the static descriptor map 
        inline static const std::shared_ptr<Descriptor<T>>& M_descriptorRef = InstancePtr();
    };
}