#pragma once
#include <string>
#include <string_view>
#include <memory>
#include <stdexcept>
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

    // [[deprecated("only available for backwards compatibility")]]
    using DotsType = Type;

    template <typename = Typeless>
    struct Descriptor;

    template <>
    struct Descriptor<Typeless>
    {
        Descriptor(Type type, std::string name, size_t size, size_t alignment);
        Descriptor(const Descriptor& other) = default;
        Descriptor(Descriptor&& other) = default;
        virtual ~Descriptor() = default;

        Descriptor& operator = (const Descriptor& rhs) = default;
        Descriptor& operator = (Descriptor&& rhs) = default;

        Type type() const;
        bool isFundamentalType() const;

        const std::string& name() const;
        size_t size() const;
        size_t alignment() const;

        virtual Typeless& construct(Typeless& value) const = 0;
        virtual Typeless& construct(Typeless& value, const Typeless& other) const = 0;
        virtual Typeless& construct(Typeless& value, Typeless&& other) const = 0;

        virtual Typeless& constructInPlace(Typeless& value) const = 0;
        virtual Typeless& constructInPlace(Typeless& value, const Typeless& other) const = 0;
        virtual Typeless& constructInPlace(Typeless& value, Typeless&& other) const = 0;

        virtual void destruct(Typeless& value) const = 0;

        virtual Typeless& assign(Typeless& lhs, const Typeless& rhs) const = 0;
        virtual Typeless& assign(Typeless& lhs, Typeless&& rhs) const = 0;
        virtual void swap(Typeless& value, Typeless& other) const = 0;

        virtual bool equal(const Typeless& lhs, const Typeless& rhs) const = 0;
        virtual bool less(const Typeless& lhs, const Typeless& rhs) const = 0;
        virtual bool lessEqual(const Typeless& lhs, const Typeless& rhs) const = 0;
        virtual bool greater(const Typeless& lhs, const Typeless& rhs) const = 0;
        virtual bool greaterEqual(const Typeless& lhs, const Typeless& rhs) const = 0;

        virtual bool usesDynamicMemory() const;
        virtual size_t dynamicMemoryUsage(const Typeless& value) const;

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
        static bool IsFundamentalType(Type type);

        [[deprecated("only available for backwards compatibility")]]
        DotsType dotsType() const
        {
            return type();
        }

        [[deprecated("only available for backwards compatibility")]]
        void* New() const
        {
            return NewInternal();
        }

        [[deprecated("only available for backwards compatibility")]]
        void Delete(void *obj) const
        {
            return DeleteInternal(obj);
        }

        [[deprecated("only available for backwards compatibility")]]
        std::shared_ptr<void> make_shared() const
        {
            return { NewInternal(), [this](void* obj){ DeleteInternal(obj); } };
        }

    private:
        
        void* NewInternal() const
        {
            void *obj = ::operator new(size());
            construct(*Typeless::From(obj));
            return obj;
        }
        
        void DeleteInternal(void *obj) const
        {
            destruct(*Typeless::From(obj));
            ::operator delete(obj);
        }

        Type m_type;
        std::string m_name;
        size_t m_size;
        size_t m_alignment;
    };

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

    [[deprecated("only available for backwards compatibility and should be replaced by fundamental type check")]]
    inline bool isDotsBaseType(Type dotsType)
    {
        switch (dotsType)
        {
            case Type::int8:
            case Type::int16:
            case Type::int32:
            case Type::int64:
            case Type::uint8:
            case Type::uint16:
            case Type::uint32:
            case Type::uint64:
            case Type::boolean:
            case Type::float32:
            case Type::float64:
            case Type::string:
            case Type::property_set:
            case Type::timepoint:
            case Type::steady_timepoint:
            case Type::duration:
            case Type::uuid:
            case Type::Enum:
                return true;

            case Type::Vector:
            case Type::Struct:
                return false;
        }

        return false;
    }
}