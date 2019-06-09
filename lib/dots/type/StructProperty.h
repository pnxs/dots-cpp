#pragma once
#include <string_view>
#include "Descriptor.h"

namespace dots::type
{
	struct StructProperty
	{
		constexpr StructProperty() :
			_offset(0),
			_tag(0),
			_isKey(false),
			_nameStr(nullptr),
			_typeDescriptor(nullptr)
		{
			/* do nothing */
		}
		constexpr StructProperty(size_t offset, uint32_t tag, bool isKey, const std::string_view& name, const std::string_view& type) :
			_offset(offset),
			_tag(tag),
			_isKey(isKey),
			_name(name),
			_type(type),
			_nameStr(nullptr),
			_typeDescriptor(nullptr)
		{
			/* do nothing */
		}
		StructProperty(const std::string &name, std::size_t offset, int tag, bool key, const Descriptor *td);

		constexpr StructProperty(const StructProperty& other) = default;
		constexpr StructProperty(StructProperty&& other) = default;
		~StructProperty() = default;

		constexpr StructProperty& operator = (const StructProperty& rhs) = default;
		constexpr StructProperty& operator = (StructProperty&& rhs) = default;

	    constexpr size_t offset() const
	    {
			return _offset;
	    }

		constexpr uint32_t tag() const
		{
			return _tag;
		}

		constexpr bool isKey() const
	    {
			return _isKey;
	    }

		constexpr const std::string_view& name() const
	    {
			return _name;
	    }

		constexpr const std::string_view& typeName() const
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
		std::string_view _name;
		std::string_view _type;
		std::string* _nameStr;
	    mutable const Descriptor* _typeDescriptor;
	};
}