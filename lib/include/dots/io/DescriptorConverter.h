#pragma once
#include <dots/type/Registry.h>
#include <EnumDescriptorData.dots.h>
#include <StructDescriptorData.dots.h>

namespace dots::io
{
    struct DescriptorConverter
    {
        DescriptorConverter(type::Registry& registry);

        type::EnumDescriptor<>& operator () (const types::EnumDescriptorData& enumData) const;
        type::StructDescriptor<>& operator () (const types::StructDescriptorData& structData) const;

        types::EnumDescriptorData operator () (const type::EnumDescriptor<>& enumDescriptor);
        types::StructDescriptorData operator () (const type::StructDescriptor<>& structDescriptor);

    private:

        std::reference_wrapper<type::Registry> m_registry;
    };
}
