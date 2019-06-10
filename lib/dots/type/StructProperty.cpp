#include "StructProperty.h"
#include "Registry.h"

namespace dots::type
{
	StructProperty::StructProperty():
		_offset(0),
		_tag(0),
		_isKey(false),
		_typeDescriptor(nullptr)
	{
		/* do nothing */
	}

	StructProperty::StructProperty(const PropertyDescription& description):
		_offset(description.offset),
		_tag(description.tag),
		_isKey(description.isKey),
		_name(description.name.data()),
		_type(description.type.data()),
		_typeDescriptor(nullptr)
	{
		/* do nothing */
	}

	StructProperty::StructProperty(std::string name, std::size_t offset, int tag, bool key, const Descriptor *td) :
		_offset(offset),
		_tag(tag),
		_isKey(key),
		_name(std::move(name)),
		_type(td->name()),		
		_typeDescriptor(td)
	{
		/* do nothing */
	}

	size_t StructProperty::offset() const
	{
		return _offset;
	}

	uint32_t StructProperty::tag() const
	{
		return _tag;
	}

	bool StructProperty::isKey() const
	{
		return _isKey;
	}

	const std::string& StructProperty::name() const
	{
		return _name;
	}

	const std::string& StructProperty::typeName() const
	{
		return _type;
	}

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