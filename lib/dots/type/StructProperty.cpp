#include "StructProperty.h"
#include "Registry.h"

namespace dots::type
{
	StructProperty::StructProperty(const std::string &name, std::size_t offset, int tag, bool key, const Descriptor *td) :
		_offset(offset),
		_tag(tag),
		_isKey(key),
		_type(td->name()),		
		_typeDescriptor(td)
	{
		_nameStr = new std::string(name);
		_name = *_nameStr;
	}

	/*StructProperty::~StructProperty()
	{
		if (_nameStr != nullptr)
		{
			delete _nameStr;
		}
	}*/

	bool StructProperty::equal(const void *lhs, const void *rhs) const
	{
	    return _typeDescriptor->equal(address(lhs), address(rhs));
	}

	bool StructProperty::lessThen(const void *lhs, const void *rhs) const
	{
	    return _typeDescriptor->lessThan(address(lhs), address(rhs));
	}

	void StructProperty::copy(void *lhs, const void *rhs) const
	{
		_typeDescriptor->copy(address(lhs), address(rhs));
	}

	void StructProperty::swap(void *lhs, void *rhs) const
	{
		_typeDescriptor->swap(address(lhs), address(rhs));
	}

	void StructProperty::clear(void *lhs) const
	{
	    _typeDescriptor->clear(address(lhs));
	}

	char* StructProperty::address(void *p) const
	{
		return reinterpret_cast<char*>((char*)p + offset());
	}

	const char* StructProperty::address(const void *p) const
	{
		return reinterpret_cast<const char*>((const char*)p + offset());
	}

	const Descriptor* StructProperty::td() const
	{
		if (_typeDescriptor == nullptr)
		{
			_typeDescriptor = Registry::fromWireName(typeName().data());
		}

		return _typeDescriptor;
	}
}