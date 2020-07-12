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
        template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
        static std::underlying_type_t<T> underlying_type(T&&);
        template <typename T, std::enable_if_t<!std::is_enum_v<T>, int> = 0>
        static T underlying_type(T&&);

        template <typename T>
        using underlying_type_t = std::decay_t<decltype(underlying_type(std::declval<T>()))>;
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

    template <typename E = Typeless, typename = void>
    struct EnumDescriptor;

    template <>
    struct EnumDescriptor<Typeless> : Descriptor<Typeless>
    {
        using enumerator_ref_t = std::reference_wrapper<EnumeratorDescriptor<>>;
        
        EnumDescriptor(std::string name, const Descriptor<Typeless>& underlyingDescriptor);
        EnumDescriptor(const EnumDescriptor& other) = default;
        EnumDescriptor(EnumDescriptor&& other) = default;
        ~EnumDescriptor() = default;

        EnumDescriptor& operator = (const EnumDescriptor& rhs) = default;
        EnumDescriptor& operator = (EnumDescriptor&& rhs) = default;

        virtual std::shared_ptr<Descriptor<>> underlyingDescriptorPtr() const = 0;
        virtual const Descriptor<Typeless>& underlyingDescriptor() const = 0;

        virtual const std::vector<enumerator_ref_t>& enumeratorsTypeless() const = 0;

        virtual const EnumeratorDescriptor<>& enumeratorFromTag(uint32_t tag) const = 0;
        virtual const EnumeratorDescriptor<>& enumeratorFromName(const std::string_view& name) const = 0;
        virtual const EnumeratorDescriptor<>& enumeratorFromValue(const Typeless& value) const = 0;

        [[deprecated("only available for backwards compatibility")]]
        const types::EnumDescriptorData& descriptorData() const;

        [[deprecated("only available for backwards compatibility")]]
        static const EnumDescriptor<>* createFromEnumDescriptorData(const types::EnumDescriptorData& sd);

    private:

        mutable const types::EnumDescriptorData* m_descriptorData = nullptr;
    };

    template <typename E>
    struct EnumDescriptor<E> : StaticDescriptor<E, EnumDescriptor<Typeless>>
    {
        using enum_t = E;
        using underlying_type_t = details::underlying_type_t<E>;
        
        EnumDescriptor(std::string name, std::vector<EnumeratorDescriptor<E>> enumeratorDescriptors) :
            StaticDescriptor<E, EnumDescriptor<Typeless>>(std::move(name), underlyingDescriptor()),
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

    template<typename T, typename = void>
    constexpr bool is_defined_v = false;
    template<typename T>
    constexpr bool is_defined_v<T, decltype(sizeof(T), void())> = true;
    template<typename T>
    using is_defined_t = std::conditional_t<is_defined_v<T>, std::true_type, std::false_type>;
    template <typename T>
    struct has_enum_type : std::conditional_t<std::conjunction_v<std::is_enum<T>, is_defined_t<Descriptor<T>>>, std::true_type, std::false_type> {};
    template <typename T>
    using has_enum_type_t = typename has_enum_type<T>::type;
    template <typename T>
    constexpr bool has_enum_type_v = has_enum_type_t<T>::value;
}

namespace dots::types
{
    template <typename E, std::enable_if_t<dots::type::has_enum_type_v<E>, int> = 0>
    std::ostream& operator << (std::ostream& os, const E& enumerator)
    {
        os << type::Descriptor<E>::Instance().enumeratorFromValue(enumerator).name();
        return os;
    }

    template <typename E, std::enable_if_t<dots::type::has_enum_type_v<E>, int> = 0>
    const std::string& to_string(const E& enumerator)
    {
        return type::Descriptor<E>::Instance().enumeratorFromValue(enumerator).name();
    }
}