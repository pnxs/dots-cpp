#pragma once
#include <string>
#include <string_view>
#include <memory>
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

        virtual void fromString(Typeless& storage, const std::string_view& value) const;
        virtual std::string toString(const Typeless& value) const;

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
            void *obj = ::operator new(size());
            construct(*Typeless::From(obj));
            return obj;
        }

        [[deprecated("only available for backwards compatibility")]]
        void Delete(void *obj) const
        {
            destruct(*Typeless::From(obj));
            ::operator delete(obj);
        }

        [[deprecated("only available for backwards compatibility")]]
        std::shared_ptr<void> make_shared() const
        {
            return { New(), [this](void* obj){ Delete(obj); } };
        }

        [[deprecated("only available for backwards compatibility")]]
        std::string to_string(const void* lhs) const
        {
            return toString(*Typeless::From(lhs));
        }

        [[deprecated("only available for backwards compatibility")]]
        bool from_string(void* lhs, const std::string& str) const
        {
            try
            {
                fromString(*Typeless::From(lhs), str);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

    private:

        Type m_type;
        std::string m_name;
        size_t m_size;
        size_t m_alignment;
    };

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
    struct is_dynamic_descriptor<T, std::void_t<typename T::dynamic_descriptor_tag_t>> : std::true_type {};

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