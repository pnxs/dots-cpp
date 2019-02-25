#pragma once
#include <string_view>
#include <array>
#include "StructProperty.h"
#include "property_set.h"

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
        Struct(const Struct& other);
        Struct(Struct&& other);
        ~Struct() = default;

        Struct& operator = (const Struct& rhs);
        Struct& operator = (Struct&& rhs);

        const StructDescriptor& _descriptor() const;

		const property_set& _keyPropertySet() const;

		property_set& _validPropertySet();
		const property_set& _validPropertySet() const;

		void _publish(const property_set& what = PROPERTY_SET_ALL, bool remove = false) const;
		void _remove(const property_set& what = PROPERTY_SET_ALL) const;

    protected:

		static constexpr uint8_t Uncached      = 0b0000'0000;
		static constexpr uint8_t Cached        = 0b0000'0001;
		static constexpr uint8_t Internal      = 0b0000'0010;
		static constexpr uint8_t Persistent    = 0b0000'0100;
		static constexpr uint8_t Cleanup       = 0b0000'1000;
		static constexpr uint8_t Local         = 0b0001'0000;
		static constexpr uint8_t SubstructOnly = 0b0010'0000;

		struct StructDescription
		{
			constexpr StructDescription(const std::string_view& name, uint8_t flags, const std::array<StructProperty, 32>& propertyDescriptions, size_t numProperties) :
				name(name), flags(flags), propertyDescriptions(propertyDescriptions), numProperties(numProperties) {}

			std::string_view name;
			uint8_t flags;
			std::array<StructProperty, 32> propertyDescriptions;
			size_t numProperties;
		};

		static const StructDescriptor* MakeStructDescriptor(StructDescriptor* structDescriptorAddr, const types::StructDescriptorData& structDescriptorData);
		static const StructDescriptor* MakeStructDescriptor(StructDescriptor* structDescriptorAddr, const StructDescription& structDescription);

    private:

		property_set _validPropSet;
        const StructDescriptor* _desc;        
    };
}