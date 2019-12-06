#include <dots/io/NewDescriptorConverter.h>
#include <dots/dots.h>

namespace dots::io
{
	NewDescriptorConverter::NewDescriptorConverter(NewRegistry& registry) :
		m_registry(std::ref(registry))
	{
		/* do nothing */
	}

	std::shared_ptr<type::NewEnumDescriptor<types::int32_t>> NewDescriptorConverter::operator () (const types::EnumDescriptorData& enumData)
	{
		if (std::shared_ptr<type::NewEnumDescriptor<>> descriptor = m_registry.get().findEnumType(*enumData.name); descriptor != nullptr)
		{
			return std::static_pointer_cast<type::NewEnumDescriptor<int32_t>>(descriptor);
		}
		
		std::vector<type::NewEnumeratorDescriptor<types::int32_t>> enumerators;

		for (const EnumElementDescriptor& enumeratorData : *enumData.elements)
		{
			enumerators.emplace_back(enumeratorData.tag, enumeratorData.name, enumeratorData.enum_value);
		}

		std::shared_ptr<type::NewEnumDescriptor<types::int32_t>> descriptor = m_registry.get().registerType(type::NewEnumDescriptor<types::int32_t>{ enumData.name, std::move(enumerators) });
		return descriptor;
	}
	
	std::shared_ptr<type::NewStructDescriptor<>> NewDescriptorConverter::operator () (const types::StructDescriptorData& structData)
	{
		if (std::shared_ptr<type::NewStructDescriptor<>> descriptor = m_registry.get().findStructType(*structData.name); descriptor != nullptr)
		{
			return descriptor;
		}
		
		type::new_property_descriptor_container_t propertyDescriptors;
		size_t alignment = alignof(type::NewStruct);
		size_t size;

		uint8_t flags = type::NewStructDescriptor<>::Uncached;

		if (structData.flags.isValid())
		{
			if (structData.flags->cached)        flags |= type::NewStructDescriptor<>::Cached;
			if (structData.flags->internal)      flags |= type::NewStructDescriptor<>::Internal;
			if (structData.flags->persistent)    flags |= type::NewStructDescriptor<>::Persistent;
			if (structData.flags->cleanup)       flags |= type::NewStructDescriptor<>::Cleanup;
			if (structData.flags->local)         flags |= type::NewStructDescriptor<>::Local;
			if (structData.flags->substructOnly) flags |= type::NewStructDescriptor<>::SubstructOnly;
		}		

		const type::NewPropertyDescriptor<>* last = nullptr;

		for (const StructPropertyData& propertyData : *structData.properties)
		{
			std::shared_ptr<type::NewDescriptor<>> descriptor = m_registry.get().findType(*propertyData.type);

			if (descriptor == nullptr)
			{
				const std::string& typeName = *propertyData.type;
				if (typeName.find("vector<") == std::string::npos)
				{
					throw std::logic_error{ "missing type dependency: " + typeName };
				}
				
				std::string valueTypeName = typeName.substr(7, typeName.size() - 8);
				const std::shared_ptr<type::NewDescriptor<>>& valueTypeDescriptor = m_registry.get().findType(valueTypeName);

				if (valueTypeDescriptor == nullptr)
				{
					throw std::logic_error{ "missing value type dependency: " + valueTypeName };
				}

				if (valueTypeDescriptor->type() == type::NewType::Enum)
				{
					descriptor = m_registry.get().registerType(type::NewDescriptor<types::vector_t<types::int32_t>>{ valueTypeDescriptor });
				}
				else if (valueTypeDescriptor->type() == type::NewType::Struct)
				{
					throw std::logic_error{ "dynamic struct vector types are currently not supported" };
				}
				else
				{
					throw std::logic_error{ "unsupported dynamic vector type: " + valueTypeName };
				}
			}
			
			if (last == nullptr)
			{
				last = propertyDescriptors.emplace_back(new type::NewPropertyDescriptor<>{ descriptor, propertyData.name, propertyData.tag, propertyData.isKey });
			}
			else
			{
				last = propertyDescriptors.emplace_back(new type::NewPropertyDescriptor<>{ descriptor, propertyData.name, *last, propertyData.tag, propertyData.isKey });
			}

			alignment = std::max(last->valueDescriptor().alignment(), alignment);
		}

		size_t currentOffset = last->offset() + last->valueDescriptor().size();
		size = sizeof(void*) + currentOffset + (alignment - (currentOffset % alignment)) % alignment;

		std::shared_ptr<type::NewStructDescriptor<>> descriptor = m_registry.get().registerType(type::NewStructDescriptor<>{ structData.name, flags, propertyDescriptors, size, alignment });

		return descriptor;
	}

	types::EnumDescriptorData NewDescriptorConverter::operator () (const type::NewEnumDescriptor<>& enumDescriptor)
	{
		EnumDescriptorData enumData{ EnumDescriptorData::name_i{ enumDescriptor.name() } };
	    types::vector_t<types::EnumElementDescriptor>& enumeratorData = enumData.elements();

		for (const type::NewEnumeratorDescriptor<>& enumeratorDescriptor : enumDescriptor.enumeratorsTypeless())
		{
	        enumeratorData.emplace_back(
	            EnumElementDescriptor::enum_value_i{ enumeratorDescriptor.valueTypeless().to<int32_t>() },
	            EnumElementDescriptor::name_i{ enumeratorDescriptor.name() },
	            EnumElementDescriptor::tag_i{ enumeratorDescriptor.tag() });
		}

		return enumData;
	}
	
	types::StructDescriptorData NewDescriptorConverter::operator () (const type::NewStructDescriptor<>& structDescriptor)
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

	    for (const type::NewPropertyDescriptor<>* propertyDescriptor : structDescriptor.propertyDescriptors())
	    {
	        StructPropertyData propertyData;
	        propertyData.tag(propertyDescriptor->tag());
	        propertyData.name(propertyDescriptor->name());
	        propertyData.isKey(propertyDescriptor->isKey());
	        propertyData.type(propertyDescriptor->valueDescriptor().name());
	        properties.emplace_back(propertyData);
	    }

		return structData;
	}
}