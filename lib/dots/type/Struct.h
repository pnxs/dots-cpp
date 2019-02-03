#pragma once
#include <string_view>
#include <array>
#include "property_set.h"
#include "Property.h"

namespace dots::types
{
	struct StructDescriptorData;
}

namespace dots::type
{
	struct StructDescriptor;
	struct StructDescription;
	struct PropertyDescription;

    struct Struct
    {
        Struct(const StructDescriptor& descriptor);
        Struct(const Struct& other) = default;
        Struct(Struct&& other) = default;
        ~Struct() = default;

        Struct& operator = (const Struct& rhs) = default;
        Struct& operator = (Struct&& rhs) = default;

        const StructDescriptor& descriptor() const;

    protected:

		struct PropertyDescription
		{
			size_t offset;
			uint32_t tag;
			bool isKey;
			std::string_view name;
			std::string_view type;
		};

		static constexpr uint8_t Cached        = 0b0000'0001;
		static constexpr uint8_t Internal      = 0b0000'0010;
		static constexpr uint8_t Persistent    = 0b0000'0100;
		static constexpr uint8_t Cleanup       = 0b0000'1000;
		static constexpr uint8_t Local         = 0b0001'0000;
		static constexpr uint8_t SubstructOnly = 0b0010'0000;

		struct StructDescription
		{
			std::string_view name;
			uint8_t flags;
			std::array<PropertyDescription, 32> propertyDescriptions;
			size_t numProperties;
		};

		static const StructDescriptor* MakeStructDescriptor(const types::StructDescriptorData& structDescriptorData);
		static const StructDescriptor* MakeStructDescriptor(const StructDescription& structDescription);

    private:

		property_set _validProperties;
        const StructDescriptor* _descriptor;        
    };
}