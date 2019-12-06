#include <dots/io/NewRegistry.h>
#include <dots/io/NewDescriptorConverter.h>
#include <dots/dots.h>

namespace dots::io
{
	NewRegistry::NewRegistry()
	{
		// ensure fundamental types are instantiated and added to static descriptor map 
		type::NewDescriptor<types::bool_t>::InstancePtr();

		type::NewDescriptor<types::int8_t>::InstancePtr();
		type::NewDescriptor<types::uint8_t>::InstancePtr();
		type::NewDescriptor<types::int16_t>::InstancePtr();
		type::NewDescriptor<types::uint16_t>::InstancePtr();
		type::NewDescriptor<types::int32_t>::InstancePtr();
		type::NewDescriptor<types::uint32_t>::InstancePtr();
		type::NewDescriptor<types::int64_t>::InstancePtr();
		type::NewDescriptor<types::uint64_t>::InstancePtr();

		type::NewDescriptor<types::float32_t>::InstancePtr();
		type::NewDescriptor<types::float64_t>::InstancePtr();

		type::NewDescriptor<types::property_set_t>::InstancePtr();

		type::NewDescriptor<types::timepoint_t>::InstancePtr();
		type::NewDescriptor<types::steady_timepoint_t>::InstancePtr();
		type::NewDescriptor<types::duration_t>::InstancePtr();

		type::NewDescriptor<types::uuid_t>::InstancePtr();
		type::NewDescriptor<types::string_t>::InstancePtr();

		// ensure fundamental vector types are instantiated and added to static descriptor map
		type::NewDescriptor<types::vector_t<types::bool_t>>::InstancePtr();
		
		type::NewDescriptor<types::vector_t<types::int8_t>>::InstancePtr();
		type::NewDescriptor<types::vector_t<types::uint8_t>>::InstancePtr();
		type::NewDescriptor<types::vector_t<types::int16_t>>::InstancePtr();
		type::NewDescriptor<types::vector_t<types::uint16_t>>::InstancePtr();
		type::NewDescriptor<types::vector_t<types::int32_t>>::InstancePtr();
		type::NewDescriptor<types::vector_t<types::uint32_t>>::InstancePtr();
		type::NewDescriptor<types::vector_t<types::int64_t>>::InstancePtr();
		type::NewDescriptor<types::vector_t<types::uint64_t>>::InstancePtr();

		type::NewDescriptor<types::vector_t<types::float32_t>>::InstancePtr();
		type::NewDescriptor<types::vector_t<types::float64_t>>::InstancePtr();

		type::NewDescriptor<types::vector_t<types::property_set_t>>::InstancePtr();

		type::NewDescriptor<types::vector_t<types::timepoint_t>>::InstancePtr();
		type::NewDescriptor<types::vector_t<types::steady_timepoint_t>>::InstancePtr();
		type::NewDescriptor<types::vector_t<types::duration_t>>::InstancePtr();

		type::NewDescriptor<types::vector_t<types::uuid_t>>::InstancePtr();
		type::NewDescriptor<types::vector_t<types::string_t>>::InstancePtr();
	}

	std::shared_ptr<type::NewDescriptor<>> NewRegistry::findType(const std::string_view& name, bool assertNotNull/* = false*/) const
	{
		if (const std::shared_ptr<type::NewDescriptor<>>& descriptor = type::NewStaticDescriptorMap::Find(name); descriptor == nullptr)
		{
			if (auto it = m_types.find(name); it == m_types.end())
			{
				if (assertNotNull)
				{
					throw std::logic_error{ std::string{ "no type registered with name: " } + name.data() };
				}
				else
				{
					return nullptr;
				}
			}
			else
			{
				return it->second;
			}			
		}
		else
		{
			return descriptor;
		}
	}

	std::shared_ptr<type::NewEnumDescriptor<>> NewRegistry::findEnumType(const std::string_view& name, bool assertNotNull/* = false*/) const
	{
		const auto& descriptor = std::static_pointer_cast<type::NewEnumDescriptor<>>(findType(name, assertNotNull));
		return descriptor == nullptr ? nullptr : (descriptor->type() == type::NewType::Enum ? descriptor : nullptr);
	}

	std::shared_ptr<type::NewStructDescriptor<>> NewRegistry::findStructType(const std::string_view& name, bool assertNotNull/* = false*/) const
	{
		const auto& descriptor = std::static_pointer_cast<type::NewStructDescriptor<>>(findType(name, assertNotNull));
		return descriptor == nullptr ? nullptr : (descriptor->type() == type::NewType::Struct ? descriptor : nullptr);
	}

	const type::NewDescriptor<>& NewRegistry::getType(const std::string_view& name) const
	{
		return *findType(name, true);
	}
	
    const type::NewEnumDescriptor<>& NewRegistry::getEnumType(const std::string_view& name) const
	{
		return *findEnumType(name, true);
	}
	
    const type:: NewStructDescriptor<>& NewRegistry::getStructType(const std::string_view& name) const
	{
		return *findStructType(name, true);
	}
	
	bool NewRegistry::hasType(const std::string_view& name) const
	{
		return findType(name) != nullptr;
	}

	std::shared_ptr<type::NewDescriptor<>> NewRegistry::registerType(std::shared_ptr<type::NewDescriptor<>> descriptor, bool assertNewType/* = true*/)
	{
		auto [it, emplaced] = m_types.try_emplace(descriptor->name(), descriptor);

		if (!emplaced)
		{
			if (assertNewType)
			{
				throw std::logic_error{ "there already is a type with name: " + descriptor->name() };
			}
			else
			{
				return it->second;
			}
		}
		
		if (descriptor->type() == type::NewType::Vector)
		{
			auto vectorDescriptor = std::static_pointer_cast<type::NewVectorDescriptor>(descriptor);
			registerType(vectorDescriptor->valueDescriptorPtr(), false);
		}
		else if (descriptor->type() == type::NewType::Enum)
		{
			auto enumDescriptor = std::static_pointer_cast<type::NewEnumDescriptor<>>(descriptor);
			registerType(enumDescriptor->underlyingDescriptorPtr(), false);
		}
		else if (descriptor->type() == type::NewType::Struct)
		{
			auto structDescriptor = std::static_pointer_cast<type::NewStructDescriptor<>>(descriptor);

			for (const type::NewPropertyDescriptor<>* propertyDescriptor : structDescriptor->propertyDescriptors())
			{
				registerType(propertyDescriptor->valueDescriptorPtr(), false);
			}
		}

		return it->second;			
	}

	void NewRegistry::deregisterType(const std::shared_ptr<type::NewDescriptor<>>& descriptor, bool assertRegisteredType/* = true*/)
	{
		deregisterType(descriptor->name(), assertRegisteredType);
	}
	
	void NewRegistry::deregisterType(const type::NewDescriptor<>& descriptor, bool assertRegisteredType/* = true*/)
	{
		deregisterType(descriptor.name(), assertRegisteredType);
	}

	void NewRegistry::deregisterType(const std::string_view& name, bool assertRegisteredType/* = true*/)
	{
		auto it = m_types.find(name);

		if (it == m_types.end())
		{
			if (assertRegisteredType)
			{
				throw std::logic_error{ std::string{ "no type registered with name: " } + name.data() };
			}
		}
		else
		{
			m_types.erase(it);
		} 
	}

	const type::NewDescriptor<>* NewRegistry::findDescriptor(const std::string& name) const
    {
	    return findType(name).get();
    }
	
    const type::NewStructDescriptor<>* NewRegistry::findStructDescriptor(const std::string& name) const
    {
	    return findStructType(name).get();
    }

	const std::map<std::string_view, std::shared_ptr<type::NewDescriptor<>>>& NewRegistry::getTypes()
    {
	    return m_types;
    }
}