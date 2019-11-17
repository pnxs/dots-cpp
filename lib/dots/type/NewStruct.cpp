#include <dots/type/NewStruct.h>
#include <dots/type/NewStructDescriptor.h>

namespace dots::type
{
    NewStruct::NewStruct(const NewStructDescriptor<>& descriptor) :
        _desc(&descriptor)
    {
        /* do nothing */
    }

    const NewStructDescriptor<>& NewStruct::_descriptor() const
    {
        return *_desc;
    }

    bool NewStruct::_usesDynamicMemory() const
    {
        return _desc->usesDynamicMemory();
    }

    size_t NewStruct::_dynamicMemoryUsage() const
    {
        return _desc->dynamicMemoryUsage(NewTypeless::From(*this));
    }

    size_t NewStruct::_staticMemoryUsage() const
    {
        return _desc->size();
    }

    size_t NewStruct::_totalMemoryUsage() const
    {
        return _staticMemoryUsage() + _dynamicMemoryUsage();
    }

    const NewPropertySet& NewStruct::_keyProperties() const
    {
        return _desc->keyProperties();
    }

    NewStruct& NewStruct::_assign(const NewStruct& other, const NewPropertySet& includedProperties/* = NewPropertySet:All*/)
    {
        return _desc->assign(*this, other, includedProperties);
    }

    NewStruct& NewStruct::_copy(const NewStruct& other, const NewPropertySet& includedProperties/* = NewPropertySet:All*/)
    {
        return _desc->copy(*this, other, includedProperties);
    }

    NewStruct& NewStruct::_merge(const NewStruct& other, const NewPropertySet& includedProperties/* = NewPropertySet:All*/)
    {
        return _desc->merge(*this, other, includedProperties);
    }

    void NewStruct::_swap(NewStruct& other, const NewPropertySet& includedProperties/* = NewPropertySet:All*/)
    {
        return _desc->swap(*this, other, includedProperties);
    }

    void NewStruct::_clear(const NewPropertySet& includedProperties/* = NewPropertySet:All*/)
    {
        _desc->clear(*this, includedProperties);
    }

    bool NewStruct::_equal(const NewStruct& rhs, const NewPropertySet& includedProperties/* = NewPropertySet:All*/) const
    {
        return _desc->equal(*this, rhs, includedProperties);
    }

    bool NewStruct::_same(const NewStruct& rhs) const
    {
	    return _desc->same(*this, rhs);
    }

    bool NewStruct::_less(const NewStruct& rhs, const NewPropertySet& includedProperties/* = NewPropertySet:All*/) const
    {
        return _desc->less(*this, rhs, includedProperties);      
    }

	bool NewStruct::_lessEqual(const NewStruct& rhs, const NewPropertySet& includedProperties/* = NewPropertySet::All*/) const
    {
	    return _desc->lessEqual(*this, rhs, includedProperties);
    }
	
    bool NewStruct::_greater(const NewStruct& rhs, const NewPropertySet& includedProperties/* = NewPropertySet::All*/) const
    {
	    return _desc->greater(*this, rhs, includedProperties);
    }
	
    bool NewStruct::_greaterEqual(const NewStruct& rhs, const NewPropertySet& includedProperties/* = NewPropertySet::All*/) const
    {
	    return _desc->greaterEqual(*this, rhs, includedProperties);
    }

    NewPropertySet NewStruct::_diffProperties(const NewStruct& other, const NewPropertySet& includedProperties/* = NewPropertySet::All*/) const
    {
        return _desc->diffProperties(*this, other, includedProperties);
    }

    bool NewStruct::_hasProperties(const NewPropertySet properties) const
    {
        return properties <= _validProperties();
    }
}