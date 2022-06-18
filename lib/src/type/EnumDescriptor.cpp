#include <dots/type/EnumDescriptor.h>
#include <dots/type/FundamentalTypes.h>

namespace dots::type
{
    EnumeratorDescriptor::EnumeratorDescriptor(uint32_t tag, std::string name, int32_t value) :
        m_tag(tag),
        m_name(std::move(name)),
        m_value(value)
    {
        /* do nothing */
    }

    const Descriptor<int32_t>& EnumeratorDescriptor::underlyingDescriptor() const
    {
        return Descriptor<int32_t>::Instance();
    }

    Descriptor<int32_t>& EnumeratorDescriptor::underlyingDescriptor()
    {
        return Descriptor<int32_t>::Instance();
    }

    EnumDescriptor::EnumDescriptor(key_t key, std::string name, std::vector<EnumeratorDescriptor> enumeratorDescriptors) :
        StaticDescriptor(key, Type::Enum, std::move(name), sizeof(int32_t), alignof(int32_t)),
        m_enumerators{ std::move(enumeratorDescriptors) }
    {
        /* do nothing */
    }

    Typeless& EnumDescriptor::construct(Typeless& value) const
    {
        return underlyingDescriptor().construct(value);
    }

    Typeless& EnumDescriptor::construct(Typeless& value, const Typeless& other) const
    {
        return underlyingDescriptor().construct(value, other);
    }

    Typeless& EnumDescriptor::construct(Typeless& value, Typeless&& other) const
    {
        return underlyingDescriptor().construct(value, std::move(other));
    }

    Typeless& EnumDescriptor::constructInPlace(Typeless& value) const
    {
        return underlyingDescriptor().constructInPlace(value);
    }

    Typeless& EnumDescriptor::constructInPlace(Typeless& value, const Typeless& other) const
    {
        return underlyingDescriptor().constructInPlace(value, other);
    }

    Typeless& EnumDescriptor::constructInPlace(Typeless& value, Typeless&& other) const
    {
        return underlyingDescriptor().constructInPlace(value, std::move(other));
    }

    void EnumDescriptor::destruct(Typeless& value) const
    {
        underlyingDescriptor().destruct(value);
    }

    Typeless& EnumDescriptor::assign(Typeless& lhs, const Typeless& rhs) const
    {
        return underlyingDescriptor().assign(lhs, rhs);
    }

    Typeless& EnumDescriptor::assign(Typeless& lhs, Typeless&& rhs) const
    {
        return underlyingDescriptor().assign(lhs, std::move(rhs));
    }

    void EnumDescriptor::swap(Typeless& value, Typeless& other) const
    {
        underlyingDescriptor().swap(value, other);
    }

    bool EnumDescriptor::equal(const Typeless& lhs, const Typeless& rhs) const
    {
        return underlyingDescriptor().equal(lhs, rhs);
    }

    bool EnumDescriptor::less(const Typeless& lhs, const Typeless& rhs) const
    {
        return underlyingDescriptor().less(lhs, rhs);
    }

    size_t EnumDescriptor::dynamicMemoryUsage(const Typeless& value) const
    {
        return underlyingDescriptor().dynamicMemoryUsage(value);
    }

    bool EnumDescriptor::usesDynamicMemory() const
    {
        return underlyingDescriptor().usesDynamicMemory();
    }

    const Descriptor<int32_t>& EnumDescriptor::underlyingDescriptor() const
    {
        return Descriptor<int32_t>::Instance();
    }

    Descriptor<int32_t>& EnumDescriptor::underlyingDescriptor()
    {
        return Descriptor<int32_t>::Instance();
    }

    const EnumeratorDescriptor& EnumDescriptor::enumeratorFromTag(uint32_t tag) const
    {
        auto it = std::find_if(m_enumerators.begin(), m_enumerators.end(), [tag](const EnumeratorDescriptor& enumeratorDescriptor)
        {
            return enumeratorDescriptor.tag() == tag;
        });

        if (it == m_enumerators.end())
        {
            throw std::logic_error{ "Enum '" + Descriptor<>::name() + "' does not have enumerator with given tag: " + std::to_string(tag) };
        }

        return *it;
    }

    const EnumeratorDescriptor& EnumDescriptor::enumeratorFromName(std::string_view name) const
    {
        auto it = std::find_if(m_enumerators.begin(), m_enumerators.end(), [&name](const EnumeratorDescriptor& enumeratorDescriptor)
        {
            return enumeratorDescriptor.name() == name;
        });

        if (it == m_enumerators.end())
        {
            throw std::logic_error{ "Enum '" + Descriptor<>::name() + "' does not have enumerator with given name: " + name.data() };
        }

        return *it;
    }

    const EnumeratorDescriptor& EnumDescriptor::enumeratorFromValue(const Typeless& value) const
    {
        return enumeratorFromValue(value.to<int32_t>());
    }

    const EnumeratorDescriptor& EnumDescriptor::enumeratorFromValue(int32_t value) const
    {
        auto it = std::find_if(m_enumerators.begin(), m_enumerators.end(), [&value](const EnumeratorDescriptor& enumeratorDescriptor)
        {
            return enumeratorDescriptor.value() == value;
        });

        if (it == m_enumerators.end())
        {
            throw std::logic_error{ "Enum '" + Descriptor<>::name() + "' does not have enumerator with given value" };
        }

        return *it;
    }
}
