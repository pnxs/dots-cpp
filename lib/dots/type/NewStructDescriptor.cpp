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

	NewTypeless& NewStructDescriptor<NewTypeless, void>::construct(NewTypeless& value) const
	{
		return NewTypeless::From(construct(value.to<NewStruct>()));
	}

	NewStruct& NewStructDescriptor<NewTypeless, void>::construct(NewStruct& instance) const
	{
		::new(static_cast<void*>(::std::addressof(instance))) NewStruct{ *this };
		return instance;
	}

	NewTypeless& NewStructDescriptor<NewTypeless, void>::construct(NewTypeless& value, const NewTypeless& other) const
	{
		return NewTypeless::From(construct(value.to<NewStruct>(), other.to<NewStruct>()));
	}

	NewStruct& NewStructDescriptor<NewTypeless, void>::construct(NewStruct& instance, const NewStruct& other) const
	{
		::new(static_cast<void*>(::std::addressof(instance))) NewStruct{ other };
		
		for (auto& property : instance._propertyRange())
        {
            property.construct(property);
        }

		return instance;
	}

	NewTypeless& NewStructDescriptor<NewTypeless, void>::construct(NewTypeless& value, NewTypeless&& other) const
	{
		return NewTypeless::From(construct(value.to<NewStruct>(), other.to<NewStruct>()));
	}

	NewStruct& NewStructDescriptor<NewTypeless, void>::construct(NewStruct& instance, NewStruct&& other) const
	{
		::new(static_cast<void*>(::std::addressof(instance))) NewStruct{ std::move(other) };
		
		for (auto& property : instance._propertyRange())
        {
            property.construct(std::move(property));
        }

		return instance;
	}
	
	void NewStructDescriptor<NewTypeless, void>::destruct(NewTypeless& value) const
	{
		NewTypeless::From(destruct(value.to<NewStruct>()));
	}

	NewStruct& NewStructDescriptor<NewTypeless, void>::destruct(NewStruct& instance) const
	{
		for (auto& property : instance._propertyRange())
        {
            property.destroy();
        }

		return instance;
	}

	NewTypeless& NewStructDescriptor<NewTypeless, void>::assign(NewTypeless& lhs, const NewTypeless& rhs) const
	{
		return NewTypeless::From(assign(lhs.to<NewStruct>(), rhs.to<NewStruct>(), NewPropertySet::All));
	}
	
	NewTypeless& NewStructDescriptor<NewTypeless, void>::assign(NewTypeless& lhs, NewTypeless&& rhs) const
	{
		return NewTypeless::From(assign(lhs.to<NewStruct>(), rhs.to<NewStruct>(), NewPropertySet::All));
	}
	
	void NewStructDescriptor<NewTypeless, void>::swap(NewTypeless& value, NewTypeless& other) const
	{
		return swap(value.to<NewStruct>(), other.to<NewStruct>(), NewPropertySet::All);
	}
	
	bool NewStructDescriptor<NewTypeless, void>::equal(const NewTypeless& lhs, const NewTypeless& rhs) const
	{
		return equal(lhs.to<NewStruct>(), rhs.to<NewStruct>(), NewPropertySet::All);
	}
	
	bool NewStructDescriptor<NewTypeless, void>::less(const NewTypeless& lhs, const NewTypeless& rhs) const
	{
		return less(lhs.to<NewStruct>(), rhs.to<NewStruct>(), NewPropertySet::All);
	}
	
	bool NewStructDescriptor<NewTypeless, void>::lessEqual(const NewTypeless& lhs, const NewTypeless& rhs) const
	{
		return lessEqual(lhs.to<NewStruct>(), rhs.to<NewStruct>(), NewPropertySet::All);
	}
	
	bool NewStructDescriptor<NewTypeless, void>::greater(const NewTypeless& lhs, const NewTypeless& rhs) const
	{
		return greater(lhs.to<NewStruct>(), rhs.to<NewStruct>(), NewPropertySet::All);
	}
	
	bool NewStructDescriptor<NewTypeless, void>::greaterEqual(const NewTypeless& lhs, const NewTypeless& rhs) const
	{
		return greaterEqual(lhs.to<NewStruct>(), rhs.to<NewStruct>(), NewPropertySet::All);
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

	NewStruct& NewStructDescriptor<NewTypeless, void>::assign(NewStruct& instance, const NewStruct& other, const NewPropertySet& includedProperties) const
	{
		NewPropertySet assignProperties = other._validProperties() ^ includedProperties;

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
	
	NewStruct& NewStructDescriptor<NewTypeless, void>::copy(NewStruct& instance, const NewStruct& other, const NewPropertySet& includedProperties) const
	{
		NewPropertySet copyProperties = (instance._validProperties() + other._validProperties()) ^ includedProperties;

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
	
	NewStruct& NewStructDescriptor<NewTypeless, void>::merge(NewStruct& instance, const NewStruct& other, const NewPropertySet& includedProperties) const
	{
		NewPropertySet mergePropertySet = other._validProperties() ^ includedProperties;
        return instance._copy(other, mergePropertySet);
	}
	
	void NewStructDescriptor<NewTypeless, void>::swap(NewStruct& instance, NewStruct& other, const NewPropertySet& includedProperties) const
	{
		for (auto&[propertyThis, propertyOther] : instance._propertyRange(other, includedProperties))
        {
            propertyThis.swap(propertyOther);
        }
	}
	
	void NewStructDescriptor<NewTypeless, void>::clear(NewStruct& instance, const NewPropertySet& includedProperties) const
	{
		for (auto& property : instance._propertyRange(includedProperties))
        {
            property.destroy();
        }
	}
	
	bool NewStructDescriptor<NewTypeless, void>::equal(const NewStruct& lhs, const NewStruct& rhs, const NewPropertySet& includedProperties) const
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
	
	bool NewStructDescriptor<NewTypeless, void>::same(const NewStruct& lhs, const NewStruct& rhs) const
	{
		return lhs._equal(rhs, lhs._keyProperties());
	}
	
	bool NewStructDescriptor<NewTypeless, void>::less(const NewStruct& lhs, const NewStruct& rhs, const NewPropertySet& includedProperties) const
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
	
	bool NewStructDescriptor<NewTypeless, void>::lessEqual(const NewStruct& lhs, const NewStruct& rhs, const NewPropertySet& includedProperties) const
	{
		return !lhs._greater(rhs, includedProperties);
	}
	
	bool NewStructDescriptor<NewTypeless, void>::greater(const NewStruct& lhs, const NewStruct& rhs, const NewPropertySet& includedProperties) const
	{
		return rhs._less(lhs, includedProperties);
	}
	
	bool NewStructDescriptor<NewTypeless, void>::greaterEqual(const NewStruct& lhs, const NewStruct& rhs, const NewPropertySet& includedProperties) const
	{
		return !lhs._less(rhs, includedProperties);
	}
	
	NewPropertySet NewStructDescriptor<NewTypeless, void>::diffProperties(const NewStruct& instance, const NewStruct& other, const NewPropertySet& includedProperties) const
	{
		NewPropertySet symmetricDiff = instance._validProperties().symmetricDifference(other._validProperties()) ^ includedProperties;
        NewPropertySet intersection = instance._validProperties() ^ other._validProperties() ^ includedProperties;

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