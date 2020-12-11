#pragma once
#include <dots/io/Registry.h>
#include <EnumDescriptorData.dots.h>
#include <StructDescriptorData.dots.h>

namespace dots::io
{
    struct DescriptorConverter
    {
        DescriptorConverter(Registry& registry);
        DescriptorConverter(const DescriptorConverter& other) = default;
        DescriptorConverter(DescriptorConverter&& other) noexcept = default;
        ~DescriptorConverter() = default;

        DescriptorConverter& operator = (const DescriptorConverter& rhs) = default;
        DescriptorConverter& operator = (DescriptorConverter&& rhs) noexcept = default;

        std::shared_ptr<type::EnumDescriptor<>> operator () (const types::EnumDescriptorData& structDescriptorData);
        std::shared_ptr<type::StructDescriptor<>> operator () (const types::StructDescriptorData& structData);

        types::EnumDescriptorData operator () (const type::EnumDescriptor<>& enumDescriptor);
        types::StructDescriptorData operator () (const type::StructDescriptor<>& structDescriptor);

    private:

        std::reference_wrapper<Registry> m_registry;
    };
}