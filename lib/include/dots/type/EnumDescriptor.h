#pragma once
#include <vector>
#include <type_traits>
#include <string_view>
#include <functional>
#include <algorithm>
#include <dots/type/Descriptor.h>
#include <dots/type/StaticDescriptor.h>

namespace dots::types
{
    struct EnumDescriptorData;
}

namespace dots::type
{
    namespace details
    {
        template <typename T>
        using type_identity = std::enable_if<true, T>;

        template<typename T, typename = void>
        struct underlying_type : type_identity<T> {};

        template<typename T>
        struct underlying_type<T, std::enable_if_t<std::is_enum_v<T>>> : type_identity<std::underlying_type_t<T>> {};

        template<typename T>
        struct underlying_type<T, std::void_t<typename T::underlying_type_t>> : type_identity<typename T::underlying_type_t> {};

        template <typename T>
        using underlying_type_t = typename underlying_type<T>::type;
    }

    template <typename E = Typeless>
    struct EnumeratorDescriptor;

    template <>
    struct EnumeratorDescriptor<Typeless>
    {
        EnumeratorDescriptor(uint32_t tag, std::string name);
        EnumeratorDescriptor(const EnumeratorDescriptor& other) = default;
        EnumeratorDescriptor(EnumeratorDescriptor&& other) = default;
        virtual ~EnumeratorDescriptor() = default;

        EnumeratorDescriptor& operator = (const EnumeratorDescriptor& rhs) = default;
        EnumeratorDescriptor& operator = (EnumeratorDescriptor&& rhs) = default;

        uint32_t tag() const;
        const std::string& name() const;
        const Typeless& value() const;

        virtual std::shared_ptr<Descriptor<>> underlyingDescriptorPtr() const = 0;
        virtual const Descriptor<Typeless>& underlyingDescriptor() const = 0;
        virtual const Typeless& valueTypeless() const = 0;

    private:

        uint32_t m_tag;
        std::string m_name;
    };

    template <typename E>
    struct EnumeratorDescriptor : EnumeratorDescriptor<Typeless>
    {
        using enum_t = E;
        using underlying_type_t = details::underlying_type_t<E>;

        EnumeratorDescriptor(uint32_t tag, std::string name, enum_t value) :
            EnumeratorDescriptor<Typeless>(tag, std::move(name)),
            m_value(std::move(value))
        {
            /* do nothing */
        }
        EnumeratorDescriptor(const EnumeratorDescriptor& other) = default;
        EnumeratorDescriptor(EnumeratorDescriptor&& other) = default;
        ~EnumeratorDescriptor() = default;

        EnumeratorDescriptor& operator = (const EnumeratorDescriptor& rhs) = default;
        EnumeratorDescriptor& operator = (EnumeratorDescriptor&& rhs) = default;

        std::shared_ptr<Descriptor<>> underlyingDescriptorPtr() const override
        {
            return Descriptor<underlying_type_t>::InstancePtr();
        }

        const Descriptor<underlying_type_t>& underlyingDescriptor() const override
        {
            return Descriptor<underlying_type_t>::Instance();
        }

        const Typeless& valueTypeless() const override
        {
            return Typeless::From(m_value);
        }

        enum_t value() const
        {
            return m_value;
        }

    private:

        enum_t m_value;
    };

    template <typename E = Typeless, bool UseStaticDescriptorOperations = false, typename = void>
    struct EnumDescriptor;

    template <>
    struct EnumDescriptor<Typeless> : Descriptor<Typeless>
    {
        using enumerator_ref_t = std::reference_wrapper<EnumeratorDescriptor<>>;

        EnumDescriptor(std::string name, size_t underlyingTypeSize, size_t underlyingTypeAlignment);
        EnumDescriptor(const EnumDescriptor& other) = default;
        EnumDescriptor(EnumDescriptor&& other) = default;
        ~EnumDescriptor() = default;

        EnumDescriptor& operator = (const EnumDescriptor& rhs) = default;
        EnumDescriptor& operator = (EnumDescriptor&& rhs) = default;

        Typeless& construct(Typeless& value) const override
        {
            return underlyingDescriptor().construct(value);
        }

        Typeless& construct(Typeless& value, const Typeless& other) const override
        {
            return underlyingDescriptor().construct(value, other);
        }

        Typeless& construct(Typeless& value, Typeless&& other) const override
        {
            return underlyingDescriptor().construct(value, std::move(other));
        }

        void destruct(Typeless& value) const override
        {
            underlyingDescriptor().destruct(value);
        }

        Typeless& assign(Typeless& lhs, const Typeless& rhs) const override
        {
            return underlyingDescriptor().assign(lhs, rhs);
        }

        Typeless& assign(Typeless& lhs, Typeless&& rhs) const override
        {
            return underlyingDescriptor().assign(lhs, std::move(rhs));
        }

        void swap(Typeless& value, Typeless& other) const override
        {
            underlyingDescriptor().swap(value, other);
        }

        bool equal(const Typeless& lhs, const Typeless& rhs) const override
        {
            return underlyingDescriptor().equal(lhs, rhs);
        }

        bool less(const Typeless& lhs, const Typeless& rhs) const override
        {
            return underlyingDescriptor().less(lhs, rhs);
        }

        bool lessEqual(const Typeless& lhs, const Typeless& rhs) const override
        {
            return underlyingDescriptor().lessEqual(lhs, rhs);
        }

        bool greater(const Typeless& lhs, const Typeless& rhs) const override
        {
            return underlyingDescriptor().greater(lhs, rhs);
        }

        bool greaterEqual(const Typeless& lhs, const Typeless& rhs) const override
        {
            return underlyingDescriptor().greaterEqual(lhs, rhs);
        }

        size_t dynamicMemoryUsage(const Typeless& value) const override
        {
            return underlyingDescriptor().dynamicMemoryUsage(value);
        }

        bool usesDynamicMemory() const override
        {
            return underlyingDescriptor().usesDynamicMemory();
        }

        virtual std::shared_ptr<Descriptor<>> underlyingDescriptorPtr() const = 0;
        virtual const Descriptor<Typeless>& underlyingDescriptor() const = 0;

        virtual const std::vector<enumerator_ref_t>& enumeratorsTypeless() const = 0;

        virtual const EnumeratorDescriptor<>& enumeratorFromTag(uint32_t tag) const = 0;
        virtual const EnumeratorDescriptor<>& enumeratorFromName(const std::string_view& name) const = 0;
        virtual const EnumeratorDescriptor<>& enumeratorFromValue(const Typeless& value) const = 0;

        #ifndef DOTS_NO_GLOBAL_TRANSCEIVER

        [[deprecated("only available for backwards compatibility")]]
        const types::EnumDescriptorData& descriptorData() const;

        [[deprecated("only available for backwards compatibility")]]
        static const EnumDescriptor<>* createFromEnumDescriptorData(const types::EnumDescriptorData& sd);

        #endif

    private:

        mutable const types::EnumDescriptorData* m_descriptorData = nullptr;
    };

    template <typename E, bool UseStaticDescriptorOperations>
    struct EnumDescriptor<E, UseStaticDescriptorOperations> : StaticDescriptor<E, EnumDescriptor<Typeless>, UseStaticDescriptorOperations>
    {
        using enum_t = E;
        using underlying_type_t = details::underlying_type_t<E>;

        EnumDescriptor(std::string name, std::vector<EnumeratorDescriptor<E>> enumeratorDescriptors) :
            StaticDescriptor<E, EnumDescriptor<Typeless>, UseStaticDescriptorOperations>(std::move(name), sizeof(underlying_type_t), alignof(underlying_type_t)),
            m_enumerators{ std::move(enumeratorDescriptors) }
        {
            for (EnumeratorDescriptor<>& enumerator : m_enumerators)
            {
                m_enumeratorsTypeless.emplace_back(std::ref(enumerator));
            }
        }
        EnumDescriptor(const EnumDescriptor& other) = delete;
        EnumDescriptor(EnumDescriptor&& other) = default;
        ~EnumDescriptor() = default;

        EnumDescriptor& operator = (const EnumDescriptor& rhs) = delete;
        EnumDescriptor& operator = (EnumDescriptor&& rhs) = default;

        std::shared_ptr<Descriptor<>> underlyingDescriptorPtr() const override
        {
            return Descriptor<underlying_type_t>::InstancePtr();
        }

        const Descriptor<underlying_type_t>& underlyingDescriptor() const override
        {
            return Descriptor<underlying_type_t>::Instance();
        }

        const std::vector<EnumDescriptor<>::enumerator_ref_t>& enumeratorsTypeless() const override
        {
            return m_enumeratorsTypeless;
        }

        const std::vector<EnumeratorDescriptor<E>>& enumerators() const
        {
            return m_enumerators;
        }

        const EnumeratorDescriptor<E>& enumeratorFromTag(uint32_t tag) const override
        {
            auto it = std::find_if(m_enumerators.begin(), m_enumerators.end(), [tag](const EnumeratorDescriptor<E>& enumeratorDescriptor)
            {
                return enumeratorDescriptor.tag() == tag;
            });

            if (it == m_enumerators.end())
            {
                throw std::logic_error{ "Enum '" + Descriptor<>::name() + "' does not have enumerator with given tag: " + std::to_string(tag) };
            }

            return *it;
        }

        const EnumeratorDescriptor<E>& enumeratorFromName(const std::string_view& name) const override
        {
            auto it = std::find_if(m_enumerators.begin(), m_enumerators.end(), [&name](const EnumeratorDescriptor<E>& enumeratorDescriptor)
            {
                return enumeratorDescriptor.name() == name;
            });

            if (it == m_enumerators.end())
            {
                throw std::logic_error{ "Enum '" + Descriptor<>::name() + "' does not have enumerator with given name: " + name.data() };
            }

            return *it;
        }

        const EnumeratorDescriptor<E>& enumeratorFromValue(const Typeless& value) const override
        {
            return enumeratorFromValue(value.to<E>());
        }

        const EnumeratorDescriptor<E>& enumeratorFromValue(const E& value) const
        {
            auto it = std::find_if(m_enumerators.begin(), m_enumerators.end(), [&value](const EnumeratorDescriptor<E>& enumeratorDescriptor)
            {
                return enumeratorDescriptor.value() == value;
            });

            if (it == m_enumerators.end())
            {
                throw std::logic_error{ "Enum '" + Descriptor<>::name() + "' does not have enumerator with given value" };
            }

            return *it;
        }

    private:

        std::vector<EnumeratorDescriptor<E>> m_enumerators;
        std::vector<EnumDescriptor<>::enumerator_ref_t> m_enumeratorsTypeless;
    };

    [[deprecated("only available for backwards compatibility")]]
    inline const EnumDescriptor<>* toEnumDescriptor(const Descriptor<>* descriptor)
    {
        return descriptor->type() == Type::Enum ? static_cast<const EnumDescriptor<>*>(descriptor) : nullptr;
    }
}

namespace dots::type
{
    template <typename T>
    struct has_enum_type : std::conditional_t<std::conjunction_v<std::is_enum<T>, has_descriptor_t<T>>, std::true_type, std::false_type> {};
    template <typename T>
    using has_enum_type_t = typename has_enum_type<T>::type;
    template <typename T>
    constexpr bool has_enum_type_v = has_enum_type_t<T>::value;
}