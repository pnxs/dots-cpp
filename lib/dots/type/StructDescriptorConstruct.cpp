#include "StructDescriptor.h"

// For rttr getter/setter function
#include "EnumDescriptor.h"
#include "VectorDescriptor.h"

#include "Registry.h"
#include <iostream>

#include "StructDescriptorData.dots.h"

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
    size_t sizeOf = sizeof(dots::property_set);
    size_t alignOf = alignof(dots::property_set);

    size_t lastPropertyOffset = sizeof(dots::property_set);

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


//using StructInternal::create_property;


namespace dots
{
namespace type
{

const StructDescriptor * StructDescriptor::createFromStructDescriptorData(const StructDescriptorData &sd)
{
    // Check if type is already registred
    {
        auto structDescriptor = Descriptor::registry().findStructDescriptor(sd.name);
        if (structDescriptor) return structDescriptor;
    }


    auto structProperties = getStructProperties(sd);


    auto newstruct = new StructDescriptor(sd, structProperties.size, structProperties.alignment);

    std::size_t lastOffset = sizeof(dots::property_set);


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
            throw std::runtime_error("missing type '" + dots_type_name + "' for property '" + *p.name +"'");
        }
        lastOffset = offset + propertyTypeDescriptor->sizeOf();
    }

    if (sd.publisherId.isValid())
    {
        newstruct->m_publisherId = sd.publisherId;
    }


    Descriptor::registry().onNewStruct(newstruct);

    return newstruct;
}


}
}