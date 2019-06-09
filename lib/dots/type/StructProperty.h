#pragma once
#include <string_view>
#include "Descriptor.h"

namespace dots::type
{
	struct PropertyDescription
	{
		constexpr PropertyDescription() :
			offset(0), tag(0), isKey(false) {}
		constexpr PropertyDescription(size_t offset, uint32_t tag, bool isKey, const std::string_view& name, const std::string_view& type) :
			offset(offset), tag(tag), isKey(isKey), name(name), type(type) {}

		size_t offset;
		uint32_t tag;
		bool isKey;
		std::string_view name;
		std::string_view type;
	};

	struct StructProperty
	{
		StructProperty() :
			_offset(0),
			_tag(0),
			_isKey(false),
			_typeDescriptor(nullptr)
		{
			/* do nothing */
		}
		StructProperty(const PropertyDescription& description) :
			_offset(description.offset),
			_tag(description.tag),
			_isKey(description.isKey),
			_name(description.name.data()),
			_type(description.type.data()),
			_typeDescriptor(nullptr)
		{
			/* do nothing */
		}
		StructProperty(std::string name, std::size_t offset, int tag, bool key, const Descriptor *td);

		StructProperty(const StructProperty& other) = default;
		StructProperty(StructProperty&& other) = default;
		~StructProperty() = default;

		StructProperty& operator = (const StructProperty& rhs) = default;
		StructProperty& operator = (StructProperty&& rhs) = default;

	    size_t offset() const
	    {
			return _offset;
	    }

		uint32_t tag() const
		{
			return _tag;
		}

		bool isKey() const
	    {
			return _isKey;
	    }

		const std::string& name() const
	    {
			return _name;
	    }

		const std::string& typeName() const
		{
			return _type;
		}

		const Descriptor* td() const;

	    bool equal(const void* lhs, const void* rhs) const;
	    bool lessThen(const void* lhs, const void* rhs) const;
	    void copy(void *lhs, const void* rhs) const;
	    void swap(void *lhs, void* rhs) const;
	    void clear(void *lhs) const;

	    char* address(void* p) const;
	    const char* address(const void* p) const;

	private:

		size_t _offset;
		uint32_t _tag;
		bool _isKey;
		std::string _name;
		std::string _type;
	    mutable const Descriptor* _typeDescriptor;
	};
}