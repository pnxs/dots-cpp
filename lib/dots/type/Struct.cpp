#include "Struct.h"
#include "StructDescriptor.h"
#include "Registry.h"
#include "dots/io/Transceiver.h"
#include <StructDescriptorData.dots.h>

struct StructProperties
{
	std::size_t size;
	std::size_t alignment;
};

static size_t evalPropertyOffset(const dots::type::Descriptor* td, size_t start)
{
	size_t align = td->alignOf();
	return start + (align - (start % align)) % align;
}

static size_t evalMaxPropertyAlignment(const StructDescriptorData &sd)
{
	size_t maxAlign = alignof(dots::property_set);

	for (auto &p : *sd.properties)
	{
		auto td = dots::type::Descriptor::registry().findDescriptor(p.type);
		size_t align = td->alignOf();
		if (align > maxAlign)
			maxAlign = align;
	}
	return maxAlign;
}

static StructProperties getStructProperties(const StructDescriptorData &sd)
{
	size_t sizeOf = sizeof(dots::type::Struct);
	size_t alignOf = alignof(dots::type::Struct);

	size_t lastPropertyOffset = sizeof(dots::type::Struct);

	for (auto &p : *sd.properties)
	{
		std::string dots_type_name = p.type;
		auto td = dots::type::Registry::fromWireName(dots_type_name);
		if (not td) {
			throw std::runtime_error("getStructProperties: missing type: " + dots_type_name);
		}

		size_t offset = evalPropertyOffset(td, lastPropertyOffset);
		lastPropertyOffset = offset + td->sizeOf();
	}

	{
		auto pointerType = dots::type::Descriptor::registry().findDescriptor("pointer");
		sizeOf = evalPropertyOffset(pointerType, lastPropertyOffset);
		alignOf = evalMaxPropertyAlignment(sd);
	}

	return { sizeOf, alignOf };
}

static uint32_t calculateMaxTagValue(const StructDescriptorData &sd)
{
	uint32_t maxValue = 0;

	for (auto& t : *sd.properties)
	{
		maxValue = std::max(*t.tag, maxValue);
	}

	return maxValue;
}

namespace dots::type
{
    Struct::Struct(const StructDescriptor& descriptor) :
        _desc(&descriptor)
    {
        /* do nothing */
    }

    const StructDescriptor& Struct::_descriptor() const
    {
        return *_desc;
    }

    const property_set& Struct::_keyPropertySet() const
    {
		return _desc->keys();
    }

    property_set& Struct::_validPropertySet()
	{
		return _validPropSet;
	}
	const property_set& Struct::_validPropertySet() const
	{
		return _validPropSet;
	}

    void Struct::_publish(const property_set& what/* = PROPERTY_SET_ALL*/, bool remove/* = false*/) const
    {
		onPublishObject->publish(&_descriptor(), this, what, remove);
    }

    void Struct::_remove() const
    {
		_publish(PROPERTY_SET_NONE, true);
    }

    const StructDescriptor* Struct::MakeStructDescriptor(const StructDescriptorData& structDescriptorData)
	{
		// Check if type is already registred
		{
			auto structDescriptor = Descriptor::registry().findStructDescriptor(structDescriptorData.name);
			if (structDescriptor) return structDescriptor;
		}


		auto structProperties = getStructProperties(structDescriptorData);


		auto newstruct = new StructDescriptor(structDescriptorData, structProperties.size, structProperties.alignment);

		std::size_t lastOffset = sizeof(Struct);


		for (const StructPropertyData &p : *newstruct->descriptorData().properties)
		{
			std::string dots_type_name = p.type; // DOTS typename
			auto td = Registry::fromWireName(dots_type_name);

			std::size_t offset = evalPropertyOffset(td, lastOffset);
			// Create Properties
			const Descriptor* propertyTypeDescriptor = td;
			if (propertyTypeDescriptor)
			{
				newstruct->m_properties.push_back(StructProperty(p.name, offset, p.tag, p.isKey, propertyTypeDescriptor));
				newstruct->m_propertySet.set(p.tag);
				if (p.isKey) {
					newstruct->m_keyProperties.set(p.tag);
				}
			}
			else
			{
				// Error, because the needed type is not found
				throw std::runtime_error("missing type '" + dots_type_name + "' for property '" + *p.name + "'");
			}
			lastOffset = offset + propertyTypeDescriptor->sizeOf();
		}

		if (structDescriptorData.publisherId.isValid())
		{
			newstruct->m_publisherId = structDescriptorData.publisherId;
		}


		Descriptor::registry().onNewStruct(newstruct);

		return newstruct;
	}

	const StructDescriptor* Struct::MakeStructDescriptor(const StructDescription& structDescription)
    {
		StructDescriptorData structDescriptorData;
		structDescriptorData.name(structDescription.name.data());

		auto& flags = structDescriptorData.flags();
		flags.cached(structDescription.flags & Cached);
		flags.internal(structDescription.flags & Internal);
		flags.persistent(structDescription.flags & Persistent);
		flags.cleanup(structDescription.flags & Cleanup);
		flags.local(structDescription.flags & Local);
		flags.substructOnly(structDescription.flags & SubstructOnly);

		auto& properties = structDescriptorData.properties();

		for (size_t i = 0; i < structDescription.numProperties; ++i)
		{
			const PropertyDescription& propertyDescription = structDescription.propertyDescriptions[i];
			StructPropertyData structPropertyData;
			structPropertyData.name(propertyDescription.name.data());
			structPropertyData.tag(propertyDescription.tag);
			structPropertyData.isKey(propertyDescription.isKey);
			structPropertyData.type(propertyDescription.type.data());
			properties.emplace_back(structPropertyData);
		}

		return MakeStructDescriptor(structDescriptorData);
    }
}
