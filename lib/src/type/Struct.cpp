#include <dots/type/Struct.h>
#include <dots/type/StructDescriptor.h>

namespace dots::type
{
    Struct::Struct(const StructDescriptor<>& descriptor) :
        _desc(&descriptor)
    {
        /* do nothing */
    }

    const StructDescriptor<>& Struct::_descriptor() const
    {
        return *_desc;
    }

    bool Struct::_usesDynamicMemory() const
    {
        return _desc->usesDynamicMemory();
    }

    size_t Struct::_dynamicMemoryUsage() const
    {
        return _desc->dynamicMemoryUsage(Typeless::From(*this));
    }

    size_t Struct::_staticMemoryUsage() const
    {
        return _desc->size();
    }

    size_t Struct::_totalMemoryUsage() const
    {
        return _staticMemoryUsage() + _dynamicMemoryUsage();
    }

    const PropertySet& Struct::_keyProperties() const
    {
        return _desc->keyProperties();
    }

    Struct& Struct::_assign(const Struct& other, const PropertySet& includedProperties/* = PropertySet:All*/)
    {
        return _desc->assign(*this, other, includedProperties);
    }

    Struct& Struct::_assign(Struct&& other, const PropertySet& includedProperties)
    {
        return _desc->assign(*this, std::move(other), includedProperties);
    }

    Struct& Struct::_copy(const Struct& other, const PropertySet& includedProperties/* = PropertySet:All*/)
    {
        return _desc->copy(*this, other, includedProperties);
    }

    Struct& Struct::_merge(const Struct& other, const PropertySet& includedProperties/* = PropertySet:All*/)
    {
        return _desc->merge(*this, other, includedProperties);
    }

    void Struct::_swap(Struct& other, const PropertySet& includedProperties/* = PropertySet:All*/)
    {
        return _desc->swap(*this, other, includedProperties);
    }

    void Struct::_clear(const PropertySet& includedProperties/* = PropertySet:All*/)
    {
        _desc->clear(*this, includedProperties);
    }

    bool Struct::_equal(const Struct& rhs, const PropertySet& includedProperties/* = PropertySet:All*/) const
    {
        return _desc->equal(*this, rhs, includedProperties);
    }

    bool Struct::_same(const Struct& rhs) const
    {
        return _desc->same(*this, rhs);
    }

    bool Struct::_less(const Struct& rhs, const PropertySet& includedProperties/* = PropertySet:All*/) const
    {
        return _desc->less(*this, rhs, includedProperties);
    }

    bool Struct::_lessEqual(const Struct& rhs, const PropertySet& includedProperties/* = PropertySet::All*/) const
    {
        return _desc->lessEqual(*this, rhs, includedProperties);
    }

    bool Struct::_greater(const Struct& rhs, const PropertySet& includedProperties/* = PropertySet::All*/) const
    {
        return _desc->greater(*this, rhs, includedProperties);
    }

    bool Struct::_greaterEqual(const Struct& rhs, const PropertySet& includedProperties/* = PropertySet::All*/) const
    {
        return _desc->greaterEqual(*this, rhs, includedProperties);
    }

    PropertySet Struct::_diffProperties(const Struct& other, const PropertySet& includedProperties/* = PropertySet::All*/) const
    {
        return _desc->diffProperties(*this, other, includedProperties);
    }
}