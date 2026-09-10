// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/io/DescriptorConverter.h>
#include <dots/type/DynamicStruct.h>
#include <dots/type/DynamicEnum.h>

namespace dots::io
{
    DescriptorConverter::DescriptorConverter(type::Registry& registry) :
        m_registry(std::ref(registry))
    {
        /* do nothing */
    }

    type::EnumDescriptor& DescriptorConverter::operator () (const EnumDescriptorData& enumData) const
    {
        if (type::EnumDescriptor* descriptor = m_registry.get().findEnumType(*enumData.name); descriptor != nullptr)
        {
            return *descriptor;
        }

        std::vector<type::EnumeratorDescriptor> enumerators;

        for (const EnumElementDescriptor& enumeratorData : *enumData.elements)
        {
            enumerators.emplace_back(*enumeratorData.tag, *enumeratorData.name, static_cast<type::DynamicEnum>(*enumeratorData.enum_value));
        }

        return m_registry.get().registerType<type::Descriptor<type::DynamicEnum>>(*enumData.name, std::move(enumerators));
    }

    type::StructDescriptor& DescriptorConverter::operator () (const StructDescriptorData& structData) const
    {
        if (type::StructDescriptor* descriptor = m_registry.get().findStructType(*structData.name); descriptor != nullptr)
        {
            return *descriptor;
        }

        type::property_descriptor_container_t propertyDescriptors;
        size_t alignment = alignof(type::Struct);

        uint8_t flags = type::StructDescriptor::Uncached;

        if (structData.flags.isValid())
        {
            if (structData.flags->cached == true)        flags |= type::StructDescriptor::Cached;
            if (structData.flags->internal == true)      flags |= type::StructDescriptor::Internal;
            if (structData.flags->persistent == true)    flags |= type::StructDescriptor::Persistent;
            if (structData.flags->cleanup == true)       flags |= type::StructDescriptor::Cleanup;
            if (structData.flags->local == true)         flags |= type::StructDescriptor::Local;
            if (structData.flags->substructOnly == true) flags |= type::StructDescriptor::SubstructOnly;
        }

        const type::PropertyDescriptor* last = nullptr;

        for (const StructPropertyData& propertyData : *structData.properties)
        {
            type::Descriptor<>* descriptor = m_registry.get().findType(*propertyData.type);

            if (descriptor == nullptr)
            {
                const std::string& typeName = *propertyData.type;
                auto resolveReference = [&](const std::string& name) -> type::Descriptor<>*
                {
                    if (auto* known = m_registry.get().findType(name)) return known;
                    if (name.size() > 14 && name.compare(0, 13, "instance_ref<") == 0 && name.back() == '>')
                    {
                        auto target = name.substr(13, name.size() - 14);
                        if (target.find_first_of("<>") != std::string::npos)
                            throw std::logic_error{"invalid instance_ref target: " + target};
                        if (auto* targetDescriptor = m_registry.get().findType(target);
                            targetDescriptor != nullptr && targetDescriptor->type() != type::Type::Struct)
                            throw std::logic_error{"instance_ref target must be a struct: " + target};
                        return &m_registry.get().registerType<type::Descriptor<type::InstanceRef>>(target);
                    }
                    return nullptr;
                };
                descriptor = resolveReference(typeName);
                if (descriptor == nullptr)
                {
                    if (typeName.find("vector<") == std::string::npos)
                    {
                        throw std::logic_error{ "missing type dependency: " + typeName };
                    }

                    std::string valueTypeName = typeName.substr(7, typeName.size() - 8);
                    type::Descriptor<>* valueTypeDescriptor = resolveReference(valueTypeName);

                    if (valueTypeDescriptor == nullptr)
                    {
                        throw std::logic_error{ "missing value type dependency: " + valueTypeName };
                    }

                    if (valueTypeDescriptor->type() == type::Type::InstanceRef)
                    {
                        // Dynamic vectors store the generic runtime representation even
                        // when a compiled typed reference descriptor is already known.
                        auto valueDescriptor = type::make_descriptor<type::Descriptor<type::InstanceRef>>(
                            static_cast<type::Descriptor<type::InstanceRef>&>(*valueTypeDescriptor).targetTypeName());
                        descriptor = &m_registry.get().registerType<type::Descriptor<vector_t<type::InstanceRef>>>(*valueDescriptor);
                    }
                    else if (valueTypeDescriptor->type() == type::Type::Enum)
                    {
                        auto& enumDescriptor = static_cast<type::Descriptor<type::DynamicEnum>&>(*valueTypeDescriptor);
                        descriptor = &m_registry.get().registerType<type::Descriptor<vector_t<type::DynamicEnum>>>(enumDescriptor);
                    }
                    else if (valueTypeDescriptor->type() == type::Type::Struct)
                    {
                        if (auto* dynStructDescriptor = valueTypeDescriptor->as<type::Descriptor<type::DynamicStruct>>(); dynStructDescriptor == nullptr)
                        {
                            const auto& staticStructDescriptor = valueTypeDescriptor->to<type::StructDescriptor>();
                            auto dynStructDescriptor_ = type::make_descriptor<type::Descriptor<type::DynamicStruct>>(staticStructDescriptor.name(), staticStructDescriptor.flags(), staticStructDescriptor.propertyDescriptors(), staticStructDescriptor.size());
                            descriptor = &m_registry.get().registerType<type::Descriptor<vector_t<type::DynamicStruct>>>(*dynStructDescriptor_, false);
                        }
                        else
                        {
                            descriptor = &m_registry.get().registerType<type::Descriptor<vector_t<type::DynamicStruct>>>(*dynStructDescriptor, false);
                        }
                    }
                    else
                    {
                        throw std::logic_error{ "unsupported dynamic vector type: " + valueTypeName };
                    }
                }
            }

            if (last == nullptr)
            {
                last = &propertyDescriptors.emplace_back(*descriptor, *propertyData.name, *propertyData.tag, *propertyData.isKey, type::PropertyOffset::First(descriptor->alignment(), sizeof(type::PropertyArea)));
            }
            else
            {
                last = &propertyDescriptors.emplace_back(*descriptor, *propertyData.name, *propertyData.tag, *propertyData.isKey, type::PropertyOffset::Next(descriptor->alignment(), last->offset(), last->valueDescriptor().size()));
            }

            alignment = std::max(last->valueDescriptor().alignment(), alignment);
        }

        size_t size = last == nullptr ? type::PropertyOffset::First(alignment, sizeof(type::PropertyArea)) :
            type::PropertyOffset::Next(alignment, last->offset(), last->valueDescriptor().size());

        return m_registry.get().registerType<type::Descriptor<type::DynamicStruct>>(*structData.name, flags, propertyDescriptors, sizeof(type::DynamicStruct) + size);
    }

    EnumDescriptorData DescriptorConverter::operator () (const type::EnumDescriptor& enumDescriptor)
    {
        EnumDescriptorData enumData{
            .name = enumDescriptor.name()
        };

        vector_t<EnumElementDescriptor>& enumeratorData = enumData.elements.emplace();

        for (const type::EnumeratorDescriptor& enumeratorDescriptor : enumDescriptor.enumeratorsTypeless())
        {
            enumeratorData.emplace_back(EnumElementDescriptor{
                .enum_value = enumeratorDescriptor.valueTypeless().to<int32_t>() ,
                .name = enumeratorDescriptor.name() ,
                .tag = enumeratorDescriptor.tag()
            });
        }

        return enumData;
    }

    StructDescriptorData DescriptorConverter::operator () (const type::StructDescriptor& structDescriptor)
    {
        StructDescriptorData structData{
            .name = structDescriptor.name() ,
            .flags = DotsStructFlags{
                .cached = structDescriptor.cached() ,
                .internal = structDescriptor.internal() ,
                .persistent = structDescriptor.persistent() ,
                .cleanup = structDescriptor.cleanup() ,
                .local = structDescriptor.local() ,
                .substructOnly = structDescriptor.substructOnly()
            }
        };

        auto& properties = structData.properties.emplace();

        for (const type::PropertyDescriptor& propertyDescriptor : structDescriptor.propertyDescriptors())
        {
            properties.emplace_back(StructPropertyData{
                .name = propertyDescriptor.name() ,
                .tag = propertyDescriptor.tag() ,
                .isKey = propertyDescriptor.isKey() ,
                .type = propertyDescriptor.valueDescriptor().name()
            });
        }

        return structData;
    }
}
