#include <dots/io/DescriptorConverter.h>
#include <dots/type/DynamicStruct.h>
#include <dots/dots.h>

namespace dots::io
{
    DescriptorConverter::DescriptorConverter(Registry& registry/* = transceiver().registry()*/) :
        m_registry(std::ref(registry))
    {
        /* do nothing */
    }

    std::shared_ptr<type::EnumDescriptor<types::int32_t>> DescriptorConverter::operator () (const types::EnumDescriptorData& enumData)
    {
        if (std::shared_ptr<type::EnumDescriptor<>> descriptor = m_registry.get().findEnumType(*enumData.name); descriptor != nullptr)
        {
            return std::static_pointer_cast<type::EnumDescriptor<int32_t>>(descriptor);
        }

        std::vector<type::EnumeratorDescriptor<types::int32_t>> enumerators;

        for (const EnumElementDescriptor& enumeratorData : *enumData.elements)
        {
            enumerators.emplace_back(enumeratorData.tag, enumeratorData.name, enumeratorData.enum_value);
        }

        std::shared_ptr<type::EnumDescriptor<types::int32_t>> descriptor = m_registry.get().registerType(type::EnumDescriptor<types::int32_t>{ enumData.name, std::move(enumerators) });
        return descriptor;
    }

    std::shared_ptr<type::StructDescriptor<>> DescriptorConverter::operator () (const types::StructDescriptorData& structData)
    {
        if (std::shared_ptr<type::StructDescriptor<>> descriptor = m_registry.get().findStructType(*structData.name); descriptor != nullptr)
        {
            return descriptor;
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
            std::shared_ptr<type::Descriptor<>> descriptor = m_registry.get().findType(*propertyData.type);

            if (descriptor == nullptr)
            {
                const std::string& typeName = *propertyData.type;
                if (typeName.find("vector<") == std::string::npos)
                {
                    throw std::logic_error{ "missing type dependency: " + typeName };
                }

                std::string valueTypeName = typeName.substr(7, typeName.size() - 8);
                const std::shared_ptr<type::Descriptor<>>& valueTypeDescriptor = m_registry.get().findType(valueTypeName);

                if (valueTypeDescriptor == nullptr)
                {
                    throw std::logic_error{ "missing value type dependency: " + valueTypeName };
                }

                if (valueTypeDescriptor->type() == type::Type::Enum)
                {
                    descriptor = m_registry.get().registerType(type::Descriptor<types::vector_t<types::int32_t>>{ valueTypeDescriptor });
                }
                else if (valueTypeDescriptor->type() == type::Type::Struct)
                {
                    auto dynStructDescriptor = std::dynamic_pointer_cast<type::Descriptor<type::DynamicStruct>>(valueTypeDescriptor);

                    if ( dynStructDescriptor == nullptr)
                    {
                        const auto& staticStructDescriptor = static_cast<const type::StructDescriptor<>&>(*valueTypeDescriptor);
                        dynStructDescriptor = std::make_shared<type::Descriptor<type::DynamicStruct>>(staticStructDescriptor.name(), staticStructDescriptor.flags(), staticStructDescriptor.propertyDescriptors(), staticStructDescriptor.size());
                    }

                    descriptor = m_registry.get().registerType(type::Descriptor<types::vector_t<type::DynamicStruct>>{ dynStructDescriptor });
                }
                else
                {
                    throw std::logic_error{ "unsupported dynamic vector type: " + valueTypeName };
                }
            }

            if (last == nullptr)
            {
                last = &propertyDescriptors.emplace_back(descriptor, propertyData.name, propertyData.tag, propertyData.isKey);
            }
            else
            {
                last = &propertyDescriptors.emplace_back(type::PropertyDescriptor{ descriptor, propertyData.name, propertyData.tag, propertyData.isKey, type::PropertyOffset<>{ descriptor->alignment(), last->offset(), last->valueDescriptor().size() } });
            }

            alignment = std::max(last->valueDescriptor().alignment(), alignment);
        }

        size_t size = type::PropertyOffset<>{ alignment, last->offset(), last->valueDescriptor().size() };
        std::shared_ptr<type::StructDescriptor<>> descriptor = m_registry.get().registerType(type::Descriptor<type::DynamicStruct>{ structData.name, flags, propertyDescriptors, sizeof(type::DynamicStruct) + size });

        return descriptor;
    }

    types::EnumDescriptorData DescriptorConverter::operator () (const type::EnumDescriptor<>& enumDescriptor)
    {
        EnumDescriptorData enumData{ EnumDescriptorData::name_i{ enumDescriptor.name() } };
        types::vector_t<types::EnumElementDescriptor>& enumeratorData = enumData.elements();

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

        auto& flags = structData.flags();
        flags.cached(structDescriptor.cached());
        flags.internal(structDescriptor.internal());
        flags.persistent(structDescriptor.persistent());
        flags.cleanup(structDescriptor.cleanup());
        flags.local(structDescriptor.local());
        flags.substructOnly(structDescriptor.substructOnly());

        auto& properties = structData.properties();

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