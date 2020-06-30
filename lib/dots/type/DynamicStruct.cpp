#include <dots/type/DynamicStruct.h>
#include <memory>

namespace dots::type
{
	DynamicStruct::DynamicStruct(const Descriptor<DynamicStruct>& descriptor) :
		Struct(descriptor),
		m_propertyArea{ std::unique_ptr<PropertyArea>{ reinterpret_cast<PropertyArea*>(::operator new(descriptor.size() - sizeof(DynamicStruct))) } }
	{
		::new(static_cast<void*>(propertyAreaGet())) PropertyArea{};
	}

	DynamicStruct::DynamicStruct(const Descriptor<DynamicStruct>& descriptor, PropertyArea* propertyArea) :
	    Struct(descriptor),
		m_propertyArea{ propertyArea }
    {
		::new(static_cast<void*>(propertyAreaGet())) PropertyArea{};
    }

	DynamicStruct::DynamicStruct(const DynamicStruct& other) :
		DynamicStruct(static_cast<const Descriptor<DynamicStruct>&>(other._descriptor()))
	{
		*this = other;
	}

    DynamicStruct::DynamicStruct(DynamicStruct&& other) :
	    DynamicStruct(static_cast<const Descriptor<DynamicStruct>&>(other._descriptor()))
    {
		*this = std::move(other);
    }

    DynamicStruct::~DynamicStruct()
	{
		if (propertyAreaGet() != nullptr)
		{
			_clear();
		}
	}
	
	DynamicStruct& DynamicStruct::operator = (const DynamicStruct& rhs)
	{
		_assign(rhs);
		return *this;
	}

    DynamicStruct& DynamicStruct::operator = (DynamicStruct&& rhs)
    {
		_assign(std::move(rhs));
		return *this;
    }

    bool DynamicStruct::operator == (const DynamicStruct& rhs) const
	{
		return _equal(rhs);
	}

	bool DynamicStruct::operator != (const DynamicStruct& rhs) const
	{
		return !(*this == rhs);
	}

	bool DynamicStruct::operator < (const DynamicStruct& rhs) const
	{
		return _less(rhs);
	}

	bool DynamicStruct::operator <= (const DynamicStruct& rhs) const
	{
		return _lessEqual(rhs);
	}

	bool DynamicStruct::operator > (const DynamicStruct& rhs) const
	{
		return _greater(rhs);
	}

	bool DynamicStruct::operator >= (const DynamicStruct& rhs) const
	{
		return _greaterEqual(rhs);
	}

	DynamicStruct& DynamicStruct::_assign(const DynamicStruct& other, const PropertySet& includedProperties/* = PropertySet::All*/)
	{
		PropertySet assignProperties = other._validProperties() ^ includedProperties;

        for (auto&[propertyThis, propertyOther] : _propertyRange(other))
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

        return *this;
	}

    DynamicStruct& DynamicStruct::_assign(DynamicStruct&& other, const PropertySet& includedProperties)
    {
		PropertySet assignProperties = other._validProperties() ^ includedProperties;

        for (auto&[propertyThis, propertyOther] : _propertyRange(other))
        {
            if (propertyThis.isPartOf(assignProperties))
            {
                propertyThis.constructOrAssign(std::move(propertyOther));
            }
            else
            {
                propertyThis.destroy();
            }
        }

        return *this;
    }

    DynamicStruct& DynamicStruct::_copy(const DynamicStruct& other, const PropertySet& includedProperties/* = PropertySet::All*/)
	{
		PropertySet copyProperties = (_validProperties() + other._validProperties()) ^ includedProperties;

        for (auto&[propertyThis, propertyOther] : _propertyRange(other, copyProperties))
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

        return *this;
	}
	
	DynamicStruct& DynamicStruct::_merge(const DynamicStruct& other, const PropertySet& includedProperties/* = PropertySet::All*/)
	{
		PropertySet mergeProperties = other._validProperties() ^ includedProperties;

        for (auto& [propertyThis, propertyOther] : _propertyRange(other, mergeProperties))
        {
			if (propertyThis.descriptor().valueDescriptor().type() == Type::Struct)
			{
			    propertyThis.constructOrValue().to<Struct>()._merge(propertyOther->to<Struct>());
			}
			else
			{
				propertyThis.constructOrAssign(propertyOther); 
			}     
        }

        return *this;
	}
	
	void DynamicStruct::_swap(DynamicStruct& other, const PropertySet& includedProperties/* = PropertySet::All*/)
	{
		for (auto&[propertyThis, propertyOther] : _propertyRange(other, includedProperties))
	    {
	        propertyThis.swap(propertyOther);
	    }
	}
	
	void DynamicStruct::_clear(const PropertySet& includedProperties/* = PropertySet::All*/)
	{
		for (auto& property : _propertyRange(includedProperties))
        {
            property.destroy();
        }
	}
	
	bool DynamicStruct::_equal(const DynamicStruct& rhs, const PropertySet& includedProperties/* = PropertySet::All*/) const
	{
		for (const auto&[propertyThis, propertyOther] : _propertyRange(rhs, includedProperties))
        {
            if (propertyThis != propertyOther)
            {
                return false;
            }
        }

        return true;
	}

	bool DynamicStruct::_same(const DynamicStruct& rhs) const
	{
		return _equal(rhs, _keyProperties());
	}

	bool DynamicStruct::_less(const DynamicStruct& rhs, const PropertySet& includedProperties/* = PropertySet::All*/) const
	{
		if (includedProperties.empty())
        {
            return false;
        }
        else
        {
            for (const auto&[propertyThis, propertyOther] : _propertyRange(rhs, includedProperties))
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

	bool DynamicStruct::_lessEqual(const DynamicStruct& rhs, const PropertySet& includedProperties) const
	{
		return !_greater(rhs, includedProperties);
	}

	bool DynamicStruct::_greater(const DynamicStruct& rhs, const PropertySet& includedProperties) const
	{
		return rhs._less(*this, includedProperties);
	}

	bool DynamicStruct::_greaterEqual(const DynamicStruct& rhs, const PropertySet& includedProperties) const
	{
		return !_less(rhs, includedProperties);
	}

	PropertySet DynamicStruct::_diffProperties(const DynamicStruct& other, const PropertySet& includedProperties/* = PropertySet::All*/) const
	{
		PropertySet symmetricDiff = _validProperties().symmetricDifference(other._validProperties()) ^ includedProperties;
        PropertySet intersection = _validProperties() ^ other._validProperties() ^ includedProperties;

        if (!intersection.empty())
        {
            for (const auto&[propertyThis, propertyOther] : _propertyRange(other, intersection))
            {
                if (propertyThis != propertyOther)
                {
                    symmetricDiff += propertyThis.descriptor().set();
                }
            }
        }

        return symmetricDiff;
	}
	
	const PropertyArea& DynamicStruct::_propertyArea() const
	{
		return *propertyAreaGet();
	}
	
	PropertyArea& DynamicStruct::_propertyArea()
	{
		return *propertyAreaGet();
	}

    const PropertyArea* DynamicStruct::propertyAreaGet() const
    {
		if (std::holds_alternative<PropertyArea*>(m_propertyArea))
		{
		    return std::get<PropertyArea*>(m_propertyArea);
		}
		else
		{
		    return std::get<std::unique_ptr<PropertyArea>>(m_propertyArea).get();
		}
    }

    PropertyArea* DynamicStruct::propertyAreaGet()
    {
		return const_cast<PropertyArea*>(std::as_const(*this).propertyAreaGet());
    }
}