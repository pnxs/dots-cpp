#include <dots/type/StructDescriptor.h>
#include <dots/type/Struct.h>
#include <dots/io/DescriptorConverter.h>
#include <dots/type/DynamicStruct.h>
namespace dots::type
{
	StructDescriptor<Typeless, void>::StructDescriptor(std::string name, uint8_t flags, const property_descriptor_container_t& propertyDescriptors, size_t areaOffset, size_t size, size_t alignment) :
		Descriptor<Typeless>(Type::Struct, std::move(name), size, alignment),
		m_flags(flags),
		m_propertyDescriptors(propertyDescriptors),
	    m_areaOffset(areaOffset),
	    m_numSubStructs(0)
	{
		for (const PropertyDescriptor& propertyDescriptor : m_propertyDescriptors)
		{
			m_properties += propertyDescriptor.set();

			if (propertyDescriptor.isKey())
			{
				m_keyProperties += propertyDescriptor.set();
			}

			if (propertyDescriptor.valueDescriptor().type() == Type::Struct)
			{
			    ++m_numSubStructs;
			}

			if (propertyDescriptor.valueDescriptor().usesDynamicMemory())
			{
				m_dynamicMemoryProperties += propertyDescriptor.set();
			}
		}
	}

	Typeless& StructDescriptor<Typeless, void>::construct(Typeless& value) const
	{
		return Typeless::From(construct(value.to<Struct>()));
	}

	Struct& StructDescriptor<Typeless, void>::construct(Struct& instance) const
	{
		::new(static_cast<void*>(::std::addressof(instance))) Struct{ *this };
		return instance;
	}

	Typeless& StructDescriptor<Typeless, void>::construct(Typeless& value, const Typeless& other) const
	{
		return Typeless::From(construct(value.to<Struct>(), other.to<Struct>()));
	}

	Struct& StructDescriptor<Typeless, void>::construct(Struct& instance, const Struct& other) const
	{
		::new(static_cast<void*>(::std::addressof(instance))) Struct{ other };
		
		for (auto& property : instance._propertyRange())
        {
            property.construct(property);
        }

		return instance;
	}

	Typeless& StructDescriptor<Typeless, void>::construct(Typeless& value, Typeless&& other) const
	{
		return Typeless::From(construct(value.to<Struct>(), other.to<Struct>()));
	}

	Struct& StructDescriptor<Typeless, void>::construct(Struct& instance, Struct&& other) const
	{
		::new(static_cast<void*>(::std::addressof(instance))) Struct{ std::move(other) };
		
		for (auto& property : instance._propertyRange())
        {
            property.construct(std::move(property));
        }

		return instance;
	}
	
	void StructDescriptor<Typeless, void>::destruct(Typeless& value) const
	{
		Typeless::From(destruct(value.to<Struct>()));
	}

	Struct& StructDescriptor<Typeless, void>::destruct(Struct& instance) const
	{
		for (auto& property : instance._propertyRange())
        {
            property.destroy();
        }

		return instance;
	}

	Typeless& StructDescriptor<Typeless, void>::assign(Typeless& lhs, const Typeless& rhs) const
	{
		return Typeless::From(assign(lhs.to<Struct>(), rhs.to<Struct>(), PropertySet::All));
	}
	
	Typeless& StructDescriptor<Typeless, void>::assign(Typeless& lhs, Typeless&& rhs) const
	{
		return Typeless::From(assign(lhs.to<Struct>(), rhs.to<Struct>(), PropertySet::All));
	}
	
	void StructDescriptor<Typeless, void>::swap(Typeless& value, Typeless& other) const
	{
		return swap(value.to<Struct>(), other.to<Struct>(), PropertySet::All);
	}
	
	bool StructDescriptor<Typeless, void>::equal(const Typeless& lhs, const Typeless& rhs) const
	{
		return equal(lhs.to<Struct>(), rhs.to<Struct>(), PropertySet::All);
	}
	
	bool StructDescriptor<Typeless, void>::less(const Typeless& lhs, const Typeless& rhs) const
	{
		return less(lhs.to<Struct>(), rhs.to<Struct>(), PropertySet::All);
	}
	
	bool StructDescriptor<Typeless, void>::lessEqual(const Typeless& lhs, const Typeless& rhs) const
	{
		return lessEqual(lhs.to<Struct>(), rhs.to<Struct>(), PropertySet::All);
	}
	
	bool StructDescriptor<Typeless, void>::greater(const Typeless& lhs, const Typeless& rhs) const
	{
		return greater(lhs.to<Struct>(), rhs.to<Struct>(), PropertySet::All);
	}
	
	bool StructDescriptor<Typeless, void>::greaterEqual(const Typeless& lhs, const Typeless& rhs) const
	{
		return greaterEqual(lhs.to<Struct>(), rhs.to<Struct>(), PropertySet::All);
	}

    size_t StructDescriptor<Typeless, void>::areaOffset() const
    {
		return m_areaOffset;
    }

    size_t StructDescriptor<Typeless, void>::numSubStructs() const
    {
		return m_numSubStructs;
    }

	bool StructDescriptor<Typeless, void>::usesDynamicMemory() const
	{
		return m_dynamicMemoryProperties.empty();
	}

	size_t StructDescriptor<Typeless, void>::dynamicMemoryUsage(const Typeless& instance) const
	{
		return dynamicMemoryUsage(instance.to<Struct>());
	}

	size_t StructDescriptor<Typeless, void>::dynamicMemoryUsage(const Struct& instance) const
	{
		if (usesDynamicMemory())
		{
			size_t dynMemUsage = 0;

			for (const ProxyProperty<>& property : instance._propertyRange(m_dynamicMemoryProperties))
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

	uint8_t StructDescriptor<Typeless, void>::flags() const
	{
		return m_flags;
	}

	bool StructDescriptor<Typeless, void>::cached() const
	{
		return static_cast<bool>(m_flags & Cached);
	}

	bool StructDescriptor<Typeless, void>::cleanup() const
	{
		return static_cast<bool>(m_flags & Cleanup);
	}

	bool StructDescriptor<Typeless, void>::local() const
	{
		return static_cast<bool>(m_flags & Local);
	}

	bool StructDescriptor<Typeless, void>::persistent() const
	{
		return static_cast<bool>(m_flags & Persistent);
	}

	bool StructDescriptor<Typeless, void>::internal() const
	{
		return static_cast<bool>(m_flags & Internal);
	}

	bool StructDescriptor<Typeless, void>::substructOnly() const
	{
		return static_cast<bool>(m_flags & SubstructOnly);
	}

    const property_descriptor_container_t& StructDescriptor<Typeless, void>::propertyDescriptors() const
	{
		return m_propertyDescriptors;
	}

    partial_property_descriptor_container_t StructDescriptor<Typeless, void>::propertyDescriptors(const PropertySet& properties) const
    {
		partial_property_descriptor_container_t partialPropertyDescriptors;

		for (const PropertyDescriptor& propertyDescriptor : m_propertyDescriptors)
		{
		    if (propertyDescriptor.set() <= properties)
		    {
		        partialPropertyDescriptors.emplace_back(propertyDescriptor);
		    }
		}

		return partialPropertyDescriptors;
    }

    const property_descriptor_container_t& StructDescriptor<Typeless, void>::flatPropertyDescriptors() const
    {
		if (m_numSubStructs == 0)
		{
		    return m_propertyDescriptors;
		}
		else
		{
			if (m_flatPropertyDescriptors.empty())
		    {
				flatPropertyDescriptors(PropertyOffset<>{ std::in_place, 0 }, sizeof(PropertyArea), m_flatPropertyDescriptors);
		    }

			return m_flatPropertyDescriptors;
		}
    }

	void StructDescriptor<Typeless, void>::flatPropertyDescriptors(PropertyOffset<> previousOffset, size_t previousSize, property_descriptor_container_t& flatPropertyDescriptors) const
    {
		for (const PropertyDescriptor& propertyDescriptor : m_propertyDescriptors)
		{
			if (propertyDescriptor.valueDescriptor().type() == Type::Struct)
			{
				const auto& subStructDescriptor = static_cast<const StructDescriptor&>(propertyDescriptor.valueDescriptor());
				const PropertyDescriptor& flatPropertyDescriptor = flatPropertyDescriptors.emplace_back(propertyDescriptor.valueDescriptorPtr(), propertyDescriptor.name(), propertyDescriptor.tag(), propertyDescriptor.isKey(), PropertyOffset<>{ subStructDescriptor.alignment(), previousOffset, previousSize });
				previousOffset = flatPropertyDescriptor.offset();
			    previousSize = subStructDescriptor.areaOffset();

				previousOffset = PropertyOffset<>{ alignof(PropertyArea), previousOffset, previousSize };
			    previousSize = sizeof(PropertyArea);

				subStructDescriptor.flatPropertyDescriptors(previousOffset, previousSize, flatPropertyDescriptors);
				previousOffset = flatPropertyDescriptors.back().offset();
				previousSize = flatPropertyDescriptors.back().valueDescriptor().size();
			}
			else
			{
			    const PropertyDescriptor& flatPropertyDescriptor = flatPropertyDescriptors.emplace_back(propertyDescriptor.valueDescriptorPtr(), propertyDescriptor.name(), propertyDescriptor.tag(), propertyDescriptor.isKey(), PropertyOffset<>{ propertyDescriptor.valueDescriptor().alignment(), previousOffset, previousSize });
			    previousOffset = flatPropertyDescriptor.offset();
			    previousSize = flatPropertyDescriptor.valueDescriptor().size();
			}
		}
    }

    property_descriptor_path_t StructDescriptor<Typeless, void>::propertyDescriptorPath(std::string_view propertyPath) const
    {
		property_descriptor_path_t path;
		propertyDescriptorPath(path, propertyPath);

		return path;
	}

    void StructDescriptor<Typeless, void>::propertyDescriptorPath(property_descriptor_path_t& path, std::string_view propertyPath) const
    {
		std::string_view::size_type delimiterPos = propertyPath.find_first_of('.');
		std::string_view propertyName = propertyPath.substr(0, delimiterPos);
		auto it = std::find_if(m_propertyDescriptors.begin(), m_propertyDescriptors.end(), [&](const PropertyDescriptor& propertyDescriptor){ return propertyDescriptor.name() == propertyName; });

		if (it == m_propertyDescriptors.end())
		{
			throw std::runtime_error{ "there is no property with name '" + std::string{ propertyName } + "'" };
		}
		
		path.emplace_back(it);

		if (delimiterPos != std::string_view::npos)
		{
			const PropertyDescriptor& propertyDescriptor = *it;

			if (propertyDescriptor.valueDescriptor().type() != Type::Struct)
			{
				throw std::runtime_error{ "property is not a substruct '" + std::string{ propertyName } + "'" };
			}

			std::string_view::size_type subPropertyPathPos = delimiterPos + 1;

			if (subPropertyPathPos > propertyPath.size())
			{
			    throw std::runtime_error{ "invalid composed property name '" + std::string{ propertyPath } + "'" };
			}

			std::string_view subPropertyPath = propertyPath.substr(subPropertyPathPos);
			static_cast<const StructDescriptor<>&>(propertyDescriptor.valueDescriptor()).propertyDescriptorPath(path, subPropertyPath);
		}
    }

    const PropertySet& StructDescriptor<Typeless, void>::properties() const
	{
		return m_properties;
	}

	const PropertySet& StructDescriptor<Typeless, void>::keyProperties() const
	{
		return m_keyProperties;
	}

	const PropertySet& StructDescriptor<Typeless, void>::keys() const
	{
		return m_keyProperties;
	}

	const PropertySet& StructDescriptor<Typeless, void>::validProperties(const void* instance) const
	{
		return propertyArea(*reinterpret_cast<const Struct*>(instance)).validProperties();
	}

	PropertySet& StructDescriptor<Typeless, void>::validProperties(void* instance) const
	{
		return propertyArea(*reinterpret_cast<Struct*>(instance)).validProperties();
	}

	const types::StructDescriptorData& StructDescriptor<Typeless, void>::descriptorData() const
	{
		if (m_descriptorData == nullptr)
		{
			m_descriptorData = new types::StructDescriptorData{ io::DescriptorConverter{}(*this) };
		}
		
		return *m_descriptorData;
	}

	const StructDescriptor<>* StructDescriptor<Typeless, void>::createFromStructDescriptorData(const types::StructDescriptorData& sd)
	{
		return io::DescriptorConverter{}(sd).get();
	}
}