#include <dots/type/StructDescriptor.h>
#include <dots/type/Struct.h>
#include <dots/io/DescriptorConverter.h>

namespace dots::type
{
	StructDescriptor<Typeless, void>::StructDescriptor(std::string name, uint8_t flags, const property_descriptor_container_t& propertyDescriptors, size_t size, size_t alignment) :
		Descriptor<Typeless>(Type::Struct, std::move(name), size, alignment),
		m_flags(flags),
		m_propertyDescriptors(propertyDescriptors)
	{
		for (const PropertyDescriptor<>* propertyDescriptor : m_propertyDescriptors)
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

	Struct& StructDescriptor<Typeless, void>::assign(Struct& instance, const Struct& other, const PropertySet& includedProperties) const
	{
		PropertySet assignProperties = other._validProperties() ^ includedProperties;

        for (auto&[propertyThis, propertyOther] : instance._propertyRange(other))
        {
            if (propertyThis.isPartOf(assignProperties))
            {
                propertyThis.constructOrAssign(propertyOther);
            }
            else
            {
                propertyThis.destroy();
            }
        }

        return instance;
	}
	
	Struct& StructDescriptor<Typeless, void>::copy(Struct& instance, const Struct& other, const PropertySet& includedProperties) const
	{
		PropertySet copyProperties = (instance._validProperties() + other._validProperties()) ^ includedProperties;

        for (auto&[propertyThis, propertyOther] : instance._propertyRange(other, copyProperties))
        {
            if (propertyOther.isValid())
            {
                propertyThis.constructOrAssign(propertyOther);
            }
            else
            {
                propertyThis.destroy();
            }            
        }

        return instance;
	}
	
	Struct& StructDescriptor<Typeless, void>::merge(Struct& instance, const Struct& other, const PropertySet& includedProperties) const
	{
		PropertySet mergePropertySet = other._validProperties() ^ includedProperties;
        return instance._copy(other, mergePropertySet);
	}
	
	void StructDescriptor<Typeless, void>::swap(Struct& instance, Struct& other, const PropertySet& includedProperties) const
	{
		for (auto&[propertyThis, propertyOther] : instance._propertyRange(other, includedProperties))
        {
            propertyThis.swap(propertyOther);
        }
	}
	
	void StructDescriptor<Typeless, void>::clear(Struct& instance, const PropertySet& includedProperties) const
	{
		for (auto& property : instance._propertyRange(includedProperties))
        {
            property.destroy();
        }
	}
	
	bool StructDescriptor<Typeless, void>::equal(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const
	{
		for (const auto&[propertyThis, propertyOther] : lhs._propertyRange(rhs, includedProperties))
        {
            if (propertyThis != propertyOther)
            {
                return false;
            }
        }

        return true;
	}
	
	bool StructDescriptor<Typeless, void>::same(const Struct& lhs, const Struct& rhs) const
	{
		return lhs._equal(rhs, lhs._keyProperties());
	}
	
	bool StructDescriptor<Typeless, void>::less(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const
	{
		if (includedProperties.empty())
        {
            return false;
        }
        else
        {
            for (const auto&[propertyThis, propertyOther] : lhs._propertyRange(rhs, includedProperties))
            {
                if (propertyThis < propertyOther)
                {
                    return true;
                }
				else if (propertyThis > propertyOther)
				{
					return false;
				}
            }

            return false;
        }
	}
	
	bool StructDescriptor<Typeless, void>::lessEqual(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const
	{
		return !lhs._greater(rhs, includedProperties);
	}
	
	bool StructDescriptor<Typeless, void>::greater(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const
	{
		return rhs._less(lhs, includedProperties);
	}
	
	bool StructDescriptor<Typeless, void>::greaterEqual(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const
	{
		return !lhs._less(rhs, includedProperties);
	}
	
	PropertySet StructDescriptor<Typeless, void>::diffProperties(const Struct& instance, const Struct& other, const PropertySet& includedProperties) const
	{
		PropertySet symmetricDiff = instance._validProperties().symmetricDifference(other._validProperties()) ^ includedProperties;
        PropertySet intersection = instance._validProperties() ^ other._validProperties() ^ includedProperties;

        if (!intersection.empty())
        {
            for (const auto&[propertyThis, propertyOther] : instance._propertyRange(other, intersection))
            {
                if (propertyThis != propertyOther)
                {
                    symmetricDiff += propertyThis.descriptor().set();
                }
            }
        }

        return symmetricDiff;
	}

	const PropertyArea& StructDescriptor<Typeless, void>::propertyArea(const Struct& instance) const
	{
		return instance._propertyArea();
	}

	PropertyArea& StructDescriptor<Typeless, void>::propertyArea(Struct& instance) const
	{
		return instance._propertyArea();
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