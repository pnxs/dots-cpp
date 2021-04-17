#undef DOTS_NO_GLOBAL_TRANSCEIVER 
#include <dots/type/EnumDescriptor.h>
#include <dots/io/DescriptorConverter.h>
#include <dots/dots.h>

namespace dots::type
{
    EnumeratorDescriptor<Typeless>::EnumeratorDescriptor(uint32_t tag, std::string name) :
        m_tag(tag),
        m_name(std::move(name))
    {
        /* do nothing */
    }

    uint32_t EnumeratorDescriptor<Typeless>::tag() const
    {
        return m_tag;
    }

    const std::string& EnumeratorDescriptor<Typeless>::name() const
    {
        return m_name;
    }

    const Typeless& EnumeratorDescriptor<Typeless>::value() const
    {
        return valueTypeless();
    }

    EnumDescriptor<Typeless, false, void>::EnumDescriptor(key_t key, std::string name, size_t underlyingTypeSize, size_t underlyingTypeAlignment) :
        Descriptor<Typeless>(key, Type::Enum, std::move(name), underlyingTypeSize, underlyingTypeAlignment)
    {
        /* do nothing */
    }

    const types::EnumDescriptorData& EnumDescriptor<Typeless, false, void>::descriptorData() const
    {
        if (m_descriptorData == nullptr)
        {
            m_descriptorData = new types::EnumDescriptorData{ io::DescriptorConverter{ dots::transceiver().registry() }(*this) };
        }

        return *m_descriptorData;
    }

    const EnumDescriptor<>* EnumDescriptor<Typeless, false, void>::createFromEnumDescriptorData(const types::EnumDescriptorData& sd)
    {
        return io::DescriptorConverter{ dots::transceiver().registry() }(sd).get();
    }
}