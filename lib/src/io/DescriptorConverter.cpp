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

    type::EnumDescriptor<>& DescriptorConverter::operator () (const types::EnumDescriptorData& enumData) const
    {
        if (type::EnumDescriptor<>* descriptor = m_registry.get().findEnumType(*enumData.name); descriptor != nullptr)
        {
            return *descriptor;
        }

        std::vector<type::EnumeratorDescriptor<type::DynamicEnum>> enumerators;

        for (const EnumElementDescriptor& enumeratorData : *enumData.elements)
        {
            enumerators.emplace_back(enumeratorData.tag, enumeratorData.name, static_cast<type::DynamicEnum>(*enumeratorData.enum_value));
        }

        return m_registry.get().registerType<type::Descriptor<type::DynamicEnum>>(enumData.name, std::move(enumerators));
    }

    type::StructDescriptor<>& DescriptorConverter::operator () (const types::StructDescriptorData& structData) const
    {
        if (type::StructDescriptor<>* descriptor = m_registry.get().findStructType(*structData.name); descriptor != nullptr)
        {
            return *descriptor;
        }

        type::property_descriptor_container_t propertyDescriptors;
        size_t alignment = alignof(type::Struct);

        uint8_t flags = type::StructDescriptor<>::Uncached;

        if (structData.flags.isValid())
        {
            if (structData.flags->cached == true)        flags |= type::StructDescriptor<>::Cached;
            if (structData.flags->internal == true)      flags |= type::StructDescriptor<>::Internal;
            if (structData.flags->persistent == true)    flags |= type::StructDescriptor<>::Persistent;
            if (structData.flags->cleanup == true)       flags |= type::StructDescriptor<>::Cleanup;
            if (structData.flags->local == true)         flags |= type::StructDescriptor<>::Local;
            if (structData.flags->substructOnly == true) flags |= type::StructDescriptor<>::SubstructOnly;
        }

        const type::PropertyDescriptor* last = nullptr;

        for (const StructPropertyData& propertyData : *structData.properties)
        {
            type::Descriptor<>* descriptor = m_registry.get().findType(*propertyData.type);

            if (descriptor == nullptr)
            {
                const std::string& typeName = *propertyData.type;
                if (typeName.find("vector<") == std::string::npos)
                {
                    throw std::logic_error{ "missing type dependency: " + typeName };
                }

                std::string valueTypeName = typeName.substr(7, typeName.size() - 8);
                type::Descriptor<>* valueTypeDescriptor = m_registry.get().findType(valueTypeName);

                if (valueTypeDescriptor == nullptr)
                {
                    throw std::logic_error{ "missing value type dependency: " + valueTypeName };
                }

                if (valueTypeDescriptor->type() == type::Type::Enum)
                {
                    auto& enumDescriptor = static_cast<type::Descriptor<type::DynamicEnum>&>(*valueTypeDescriptor);
                    descriptor = &m_registry.get().registerType<type::Descriptor<types::vector_t<type::DynamicEnum>>>(enumDescriptor);
                }
                else if (valueTypeDescriptor->type() == type::Type::Struct)
                {
                    if (auto* dynStructDescriptor = valueTypeDescriptor->as<type::Descriptor<type::DynamicStruct>>(); dynStructDescriptor == nullptr)
                    {
                        const auto& staticStructDescriptor = valueTypeDescriptor->to<type::StructDescriptor<>>();
                        auto dynStructDescriptor_ = type::make_descriptor<type::Descriptor<type::DynamicStruct>>(staticStructDescriptor.name(), staticStructDescriptor.flags(), staticStructDescriptor.propertyDescriptors(), staticStructDescriptor.size());
                        descriptor = &m_registry.get().registerType<type::Descriptor<types::vector_t<type::DynamicStruct>>>(*dynStructDescriptor_, false);
                    }
                    else
                    {
                        descriptor = &m_registry.get().registerType<type::Descriptor<types::vector_t<type::DynamicStruct>>>(*dynStructDescriptor, false);
                    }
                }
                else
                {
                    throw std::logic_error{ "unsupported dynamic vector type: " + valueTypeName };
                }
            }

            if (last == nullptr)
            {
                last = &propertyDescriptors.emplace_back(*descriptor, propertyData.name, propertyData.tag, propertyData.isKey, type::PropertyOffset::First(descriptor->alignment(), sizeof(type::PropertyArea)));
            }
            else
            {
                last = &propertyDescriptors.emplace_back(*descriptor, propertyData.name, propertyData.tag, propertyData.isKey, type::PropertyOffset::Next(descriptor->alignment(), last->offset(), last->valueDescriptor().size()));
            }

            alignment = std::max(last->valueDescriptor().alignment(), alignment);
        }

        size_t size = type::PropertyOffset::Next(alignment, last->offset(), last->valueDescriptor().size());

        return m_registry.get().registerType<type::Descriptor<type::DynamicStruct>>(structData.name, flags, propertyDescriptors, sizeof(type::DynamicStruct) + size);
    }

    types::EnumDescriptorData DescriptorConverter::operator () (const type::EnumDescriptor<>& enumDescriptor)
    {
        EnumDescriptorData enumData{ EnumDescriptorData::name_i{ enumDescriptor.name() } };
        types::vector_t<types::EnumElementDescriptor>& enumeratorData = enumData.elements.construct();

        for (const type::EnumeratorDescriptor<>& enumeratorDescriptor : enumDescriptor.enumeratorsTypeless())
        {
            enumeratorData.emplace_back(
                EnumElementDescriptor::enum_value_i{ enumeratorDescriptor.valueTypeless().to<int32_t>() },
                EnumElementDescriptor::name_i{ enumeratorDescriptor.name() },
                EnumElementDescriptor::tag_i{ enumeratorDescriptor.tag() });
        }

        return enumData;
    }

    types::StructDescriptorData DescriptorConverter::operator () (const type::StructDescriptor<>& structDescriptor)
    {
        StructDescriptorData structData;
        structData.name(structDescriptor.name());

        auto& flags = structData.flags.construct();
        flags.cached(structDescriptor.cached());
        flags.internal(structDescriptor.internal());
        flags.persistent(structDescriptor.persistent());
        flags.cleanup(structDescriptor.cleanup());
        flags.local(structDescriptor.local());
        flags.substructOnly(structDescriptor.substructOnly());

        auto& properties = structData.properties.construct();

        for (const type::PropertyDescriptor& propertyDescriptor : structDescriptor.propertyDescriptors())
        {
            StructPropertyData propertyData;
            propertyData.tag(propertyDescriptor.tag());
            propertyData.name(propertyDescriptor.name());
            propertyData.isKey(propertyDescriptor.isKey());
            propertyData.type(propertyDescriptor.valueDescriptor().name());
            properties.emplace_back(propertyData);
        }

        return structData;
    }
}
