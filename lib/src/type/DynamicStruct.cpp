#include <dots/type/DynamicStruct.h>
#include <dots/type/FundamentalTypes.h>
#include <memory>

namespace dots::type
{
    DynamicStruct::DynamicStruct(const Descriptor<DynamicStruct>& descriptor) :
        Struct(descriptor),
        m_propertyArea{ std::unique_ptr<PropertyArea>{ static_cast<PropertyArea*>(::operator new(descriptor.size() - sizeof(DynamicStruct))) } }
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
        DynamicStruct(other._descriptor().to<Descriptor<DynamicStruct>, true>())
    {
        *this = other;
    }

    DynamicStruct::DynamicStruct(DynamicStruct&& other) :
        DynamicStruct(other._descriptor().to<Descriptor<DynamicStruct>, true>())
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

    DynamicStruct& DynamicStruct::_assign(const DynamicStruct& other, PropertySet includedProperties/* = PropertySet::All*/)
    {
        return static_cast<DynamicStruct&>(Struct::_assign(other, includedProperties));
    }

    DynamicStruct& DynamicStruct::_assign(DynamicStruct&& other, PropertySet includedProperties)
    {
        return static_cast<DynamicStruct&>(Struct::_assign(std::move(other), includedProperties));
    }

    DynamicStruct& DynamicStruct::_copy(const DynamicStruct& other, PropertySet includedProperties/* = PropertySet::All*/)
    {
        return static_cast<DynamicStruct&>(Struct::_copy(other, includedProperties));
    }

    DynamicStruct& DynamicStruct::_merge(const DynamicStruct& other, PropertySet includedProperties/* = PropertySet::All*/)
    {
        return static_cast<DynamicStruct&>(Struct::_merge(other, includedProperties));
    }

    void DynamicStruct::_swap(DynamicStruct& other, PropertySet includedProperties/* = PropertySet::All*/)
    {
        Struct::_swap(other, includedProperties);
    }

    void DynamicStruct::_clear(PropertySet includedProperties/* = PropertySet::All*/)
    {
        Struct::_clear(includedProperties);
    }

    bool DynamicStruct::_equal(const DynamicStruct& rhs, PropertySet includedProperties/* = PropertySet::All*/) const
    {
        return Struct::_equal(rhs, includedProperties);
    }

    bool DynamicStruct::_same(const DynamicStruct& rhs) const
    {
        return Struct::_same(rhs);
    }

    bool DynamicStruct::_less(const DynamicStruct& rhs, PropertySet includedProperties/* = PropertySet::All*/) const
    {
        return Struct::_less(rhs, includedProperties);
    }

    bool DynamicStruct::_lessEqual(const DynamicStruct& rhs, PropertySet includedProperties/* = PropertySet::All*/) const
    {
        return Struct::_lessEqual(rhs, includedProperties);
    }

    bool DynamicStruct::_greater(const DynamicStruct& rhs, PropertySet includedProperties/* = PropertySet::All*/) const
    {
        return Struct::_greater(rhs, includedProperties);
    }

    bool DynamicStruct::_greaterEqual(const DynamicStruct& rhs, PropertySet includedProperties/* = PropertySet::All*/) const
    {
        return Struct::_greaterEqual(rhs, includedProperties);
    }

    PropertySet DynamicStruct::_diffProperties(const DynamicStruct& other, PropertySet includedProperties/* = PropertySet::All*/) const
    {
        return Struct::_diffProperties(other, includedProperties);
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