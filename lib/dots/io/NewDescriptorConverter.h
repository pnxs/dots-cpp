#pragma once
#include <dots/io/NewRegistry.h>
#include <dots/dots.h>
#include <EnumDescriptorData.dots.h>
#include <StructDescriptorData.dots.h>

namespace dots::io
{
    struct NewDescriptorConverter
    {
        NewDescriptorConverter(NewRegistry& registry);
        NewDescriptorConverter(const NewDescriptorConverter& other) = default;
        NewDescriptorConverter(NewDescriptorConverter&& other) noexcept = default;
        ~NewDescriptorConverter() = default;

        NewDescriptorConverter& operator = (const NewDescriptorConverter& rhs) = default;
        NewDescriptorConverter& operator = (NewDescriptorConverter&& rhs) noexcept = default;

    	std::shared_ptr<type::NewEnumDescriptor<types::int32_t>> operator () (const types::EnumDescriptorData& structDescriptorData);
		std::shared_ptr<type::NewStructDescriptor<>> operator () (const types::StructDescriptorData& structData);

		types::EnumDescriptorData operator () (const type::NewEnumDescriptor<>& enumDescriptor);
		types::StructDescriptorData operator () (const type::NewStructDescriptor<>& structDescriptor);

    private:

		std::reference_wrapper<NewRegistry> m_registry;
	};
}