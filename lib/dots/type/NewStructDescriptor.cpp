#include <dots/type/NewStructDescriptor.h>
#include <dots/type/NewStruct.h>

namespace dots::type
{
	NewStructDescriptor<NewTypeless, void>::NewStructDescriptor(std::string name, uint8_t flags, const new_property_descriptor_container_t& propertyDescriptors, size_t size, size_t alignment) :
		NewDescriptor<NewTypeless>(NewType::Struct, std::move(name), size, alignment),
		m_flags(flags),
		m_propertyDescriptors(propertyDescriptors)
	{
		for (const NewPropertyDescriptor<>* propertyDescriptor : m_propertyDescriptors)
		{
			if (propertyDescriptor != nullptr)
			{
				m_properties += propertyDescriptor->set();

				if (propertyDescriptor->isKey())
				{
					m_keyProperties += propertyDescriptor->set();
				}

				if (propertyDescriptor->valueDescriptor().usesDynamicMemory())
				{
					m_dynamicMemoryProperties += propertyDescriptor->set();
				}
			}
		}
	}
	
	bool NewStructDescriptor<NewTypeless, void>::usesDynamicMemory() const
	{
		return m_dynamicMemoryProperties.empty();
	}

	size_t NewStructDescriptor<NewTypeless, void>::dynamicMemoryUsage(const NewTypeless& instance) const
	{
		return dynamicMemoryUsage(instance.to<NewStruct>());
	}

	size_t NewStructDescriptor<NewTypeless, void>::dynamicMemoryUsage(const NewStruct& instance) const
	{
		if (usesDynamicMemory())
		{
			size_t dynMemUsage = 0;

			for (const NewProxyProperty<>& property : instance._propertyRange(m_dynamicMemoryProperties))
			{
				dynMemUsage += property.descriptor().valueDescriptor().dynamicMemoryUsage(property);
			}

			return dynMemUsage;
		}
		else
		{
			return 0;
		}
	}

	uint8_t NewStructDescriptor<NewTypeless, void>::flags() const
	{
		return m_flags;
	}

	bool NewStructDescriptor<NewTypeless, void>::cached() const
	{
		return static_cast<bool>(m_flags & Cached);
	}

	bool NewStructDescriptor<NewTypeless, void>::cleanup() const
	{
		return static_cast<bool>(m_flags & Cleanup);
	}

	bool NewStructDescriptor<NewTypeless, void>::local() const
	{
		return static_cast<bool>(m_flags & Local);
	}

	bool NewStructDescriptor<NewTypeless, void>::persistent() const
	{
		return static_cast<bool>(m_flags & Persistent);
	}

	bool NewStructDescriptor<NewTypeless, void>::internal() const
	{
		return static_cast<bool>(m_flags & Internal);
	}

	bool NewStructDescriptor<NewTypeless, void>::substructOnly() const
	{
		return static_cast<bool>(m_flags & SubstructOnly);
	}

	const new_property_descriptor_container_t& NewStructDescriptor<NewTypeless, void>::propertyDescriptors() const
	{
		return m_propertyDescriptors;
	}

	const NewPropertySet& NewStructDescriptor<NewTypeless, void>::properties() const
	{
		return m_properties;
	}

	const NewPropertySet& NewStructDescriptor<NewTypeless, void>::keyProperties() const
	{
		return m_properties;
	}
}