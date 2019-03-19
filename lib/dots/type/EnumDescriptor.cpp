#include "EnumDescriptor.h"
#include "Registry.h"
#include <string.h>
#include "EnumDescriptorData.dots.h"

namespace dots
{
namespace type
{

enum class DummyTestEnum
{
    v1 = 1,
    v2 = 2
};

EnumDescriptor::EnumDescriptor(const EnumDescriptorData& ed)
:Descriptor(ed.name, DotsType::Enum, sizeof(DummyTestEnum), alignof(DummyTestEnum))
{
    m_descriptorData = std::make_shared<DescriptorData>(ed);
}


const EnumDescriptor * EnumDescriptor::createFromEnumDescriptorData(const EnumDescriptorData &ed)
{
    // Check if type is already registred
    {
        auto enumDescriptor = toEnumDescriptor(Descriptor::registry().findDescriptor(ed.name));
        if (enumDescriptor) return enumDescriptor;
    }

    auto newEnum = new EnumDescriptor(ed);

    for (auto& e : *ed.elements)
    {
        newEnum->m_elements[e.tag] = e;
    }

    return newEnum;
}

void EnumDescriptor::construct(void *) const
{
}

void EnumDescriptor::destruct(void *) const
{
}

std::string EnumDescriptor::to_string(const void* lhs) const
{

    return m_elements.at(value2key(to_int(lhs))).name;
}

bool EnumDescriptor::from_string(void* lhs, const std::string& str) const
{
    for (const auto& i : m_elements)
    {
        if (i.second.name == str)
        {
            from_int(lhs, i.second.enum_value);
            return true;
        }
    }
    return false;
}

bool EnumDescriptor::equal(const void *lhs, const void *rhs) const
{
    return to_int(lhs) == to_int(rhs);
}

bool EnumDescriptor::lessThan(const void *lhs, const void *rhs) const
{
    return to_int(lhs) < to_int(rhs);
}

void EnumDescriptor::copy(void *lhs, const void *rhs) const
{
    memcpy(lhs, rhs, sizeOf());
}

void EnumDescriptor::swap(void *lhs, void *rhs) const
{
    std::swap(to_int(lhs), to_int(rhs));
}

EnumDescriptor::enum_type EnumDescriptor::to_int(const void *p) const
{
    return *(const enum_type*)p;
}

EnumDescriptor::enum_type &EnumDescriptor::to_int(void *p) const
{
    return *(enum_type*)p;
}


void EnumDescriptor::from_int(void *p, int intValue) const
{
    *(enum_type*)p = intValue;
}

int32_t EnumDescriptor::value2key(EnumDescriptor::enum_type value) const
{
    //if (is_linear() { return value; });

    for (const auto& item : m_elements)
    {
        if (item.second.enum_value == value)
            return item.second.tag;
    }

    return 0;
}

void EnumDescriptor::clear(void *lhs) const
{
    from_int(lhs, m_elements.begin()->second.enum_value);
}

const EnumDescriptor::DescriptorData &EnumDescriptor::descriptorData() const
{
    return *(m_descriptorData.get());
}

void EnumDescriptor::from_key(void *p, int32_t key) const
{
    auto iter = m_elements.find(key);
    if (iter == m_elements.end()) {
        throw std::runtime_error("EnumDescriptor::from_key (" + name() + ") invalid key:" + std::to_string(key));
    }
    from_int(p, iter->second.enum_value);
}

int32_t EnumDescriptor::validValue() const
{
    return m_elements.at(0).enum_value;
}

}
}