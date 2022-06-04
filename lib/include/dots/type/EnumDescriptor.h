#pragma once
#include <vector>
#include <type_traits>
#include <string_view>
#include <functional>
#include <algorithm>
#include <dots/type/Descriptor.h>
#include <dots/type/StaticDescriptor.h>

namespace dots::type
{
    struct EnumeratorDescriptor
    {
        EnumeratorDescriptor(uint32_t tag, std::string name, int32_t value);

        template <typename E>
        EnumeratorDescriptor(uint32_t tag, std::string name, E value) :
            EnumeratorDescriptor(tag, std::move(name), static_cast<int32_t>(value))
        {
            static_assert(std::is_enum_v<E>);
            static_assert(std::is_same_v<std::underlying_type_t<E>, int32_t>);
        }

        EnumeratorDescriptor(const EnumeratorDescriptor& other) = default;
        EnumeratorDescriptor(EnumeratorDescriptor&& other) = default;
        virtual ~EnumeratorDescriptor() = default;

        EnumeratorDescriptor& operator = (const EnumeratorDescriptor& rhs) = default;
        EnumeratorDescriptor& operator = (EnumeratorDescriptor&& rhs) = default;

        uint32_t tag() const
        {
            return m_tag;
        }

        const std::string& name() const
        {
            return m_name;
        }

        template <typename T = int32_t>
        decltype(auto) value() const
        {
            if constexpr (std::is_same_v<T, Typeless>)
            {
                return valueTypeless();
            }
            else
            {
                return T{ m_value };
            }
        }

        const Typeless& valueTypeless() const
        {
            return Typeless::From(m_value);
        }

        const Descriptor<int32_t>& underlyingDescriptor() const;
        Descriptor<int32_t>& underlyingDescriptor();

    private:

        uint32_t m_tag;
        std::string m_name;
        int32_t m_value;
    };

    template <typename E = Typeless>
    struct EnumDescriptor;

    template <>
    struct EnumDescriptor<> : StaticDescriptor
    {
        using enumerator_ref_t = std::reference_wrapper<EnumeratorDescriptor>;

        EnumDescriptor(key_t key, std::string name, std::vector<EnumeratorDescriptor> enumeratorDescriptors);
        EnumDescriptor(const EnumDescriptor& other) = delete;
        EnumDescriptor(EnumDescriptor&& other) = delete;
        ~EnumDescriptor() override = default;

        using StaticDescriptor::construct;
        using StaticDescriptor::constructInPlace;
        using StaticDescriptor::destruct;
        using StaticDescriptor::assign;
        using StaticDescriptor::swap;
        using StaticDescriptor::equal;
        using StaticDescriptor::less;
        using StaticDescriptor::lessEqual;
        using StaticDescriptor::greater;
        using StaticDescriptor::greaterEqual;
        using StaticDescriptor::dynamicMemoryUsage;

        EnumDescriptor& operator = (const EnumDescriptor& rhs) = delete;
        EnumDescriptor& operator = (EnumDescriptor&& rhs) = delete;

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

        size_t dynamicMemoryUsage(const Typeless& value) const override;
        bool usesDynamicMemory() const override;

        const Descriptor<int32_t>& underlyingDescriptor() const;
        Descriptor<int32_t>& underlyingDescriptor();

        const std::vector<EnumeratorDescriptor>& enumeratorsTypeless() const
        {
            return m_enumerators;
        }

        const std::vector<EnumeratorDescriptor>& enumerators() const
        {
            return m_enumerators;
        }

        const EnumeratorDescriptor& enumeratorFromTag(uint32_t tag) const;
        const EnumeratorDescriptor& enumeratorFromName(std::string_view name) const;

        const EnumeratorDescriptor& enumeratorFromValue(const Typeless& value) const;
        const EnumeratorDescriptor& enumeratorFromValue(int32_t value) const;

        template <typename E, std::enable_if_t<std::is_enum_v<E>, int> = 0>
        const EnumeratorDescriptor& enumeratorFromValue(E value) const
        {
            return enumeratorFromValue(static_cast<int32_t>(value));
        }

    private:

        std::vector<EnumeratorDescriptor> m_enumerators;
    };

    template <typename TDescriptor>
    struct type_category<TDescriptor, std::enable_if_t<std::is_same_v<EnumDescriptor<>, TDescriptor>>> : std::integral_constant<Type, Type::Enum> {};
}

namespace dots::type
{
    template <typename T>
    struct is_enum : std::conjunction<std::is_enum<T>, has_descriptor_t<T>> {};
    template <typename T>
    using is_enum_t = typename is_enum<T>::type;
    template <typename T>
    constexpr bool is_enum_v = is_enum_t<T>::value;
}
