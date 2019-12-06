#include <dots/type/NewPropertyDescriptor.h>
#include <dots/type/NewPropertyArea.h>
#include <dots/type/NewStruct.h>

namespace dots::type
{
	PropertyDescriptor<Typeless, void>::PropertyDescriptor(const std::shared_ptr<Descriptor<>>& descriptor, std::string name, size_t offset, uint32_t tag, bool isKey):
		m_descriptor(descriptor),
		m_name(std::move(name)),
		m_offset(offset),
		m_tag(tag),
		m_isKey(isKey),
		m_set{PropertySet::FromIndex(m_tag)}
	{
		/* do nothing */
	}

	PropertyDescriptor<Typeless, void>::PropertyDescriptor(const std::shared_ptr<Descriptor<>>& descriptor, std::string name, uint32_t tag, bool isKey):
		PropertyDescriptor(descriptor, std::move(name), CalculateOffset(*descriptor, 0, sizeof(PropertyArea)), tag, isKey)
	{
		/* do nothing */
	}

	PropertyDescriptor<Typeless, void>::PropertyDescriptor(const std::shared_ptr<Descriptor<>>& descriptor, std::string name, const PropertyDescriptor<>& previous, uint32_t tag, bool isKey):
		PropertyDescriptor(descriptor, std::move(name), CalculateOffset(*descriptor, previous), tag, isKey)
	{
		/* do nothing */
	}

	const std::shared_ptr<Descriptor<>>& PropertyDescriptor<Typeless, void>::valueDescriptorPtr() const
	{
		return m_descriptor;
	}

	const Descriptor<>& PropertyDescriptor<Typeless, void>::valueDescriptor() const
	{
		return *m_descriptor;
	}

	const std::string& PropertyDescriptor<Typeless, void>::name() const
	{
		return m_name;
	}

	size_t PropertyDescriptor<Typeless, void>::offset() const
	{
		return m_offset;
	}

	uint32_t PropertyDescriptor<Typeless, void>::tag() const
	{
		return m_tag;
	}

	bool PropertyDescriptor<Typeless, void>::isKey() const
	{
		return m_isKey;
	}

	PropertySet PropertyDescriptor<Typeless, void>::set() const
	{
		return m_set;
	}

	char* PropertyDescriptor<Typeless, void>::address(void* p) const
	{
		return reinterpret_cast<char*>(&reinterpret_cast<Struct*>(p)->_propertyArea()) + offset();
	}

	const char* PropertyDescriptor<Typeless, void>::address(const void* p) const
	{
		return reinterpret_cast<const char*>(&reinterpret_cast<const Struct*>(p)->_propertyArea()) + offset();
	}

	size_t PropertyDescriptor<Typeless, void>::CalculateOffset(const Descriptor<>& descriptor, const PropertyDescriptor<>& previous)
	{
		return CalculateOffset(descriptor, previous.offset(), previous.valueDescriptor().size());
	}

	size_t PropertyDescriptor<Typeless, void>::CalculateOffset(const Descriptor<>& descriptor, size_t previousOffset, size_t previousSize)
	{
		size_t currentOffset = previousOffset + previousSize;
		size_t alignment = descriptor.alignment();
		size_t alignedOffset = currentOffset + (alignment - currentOffset % alignment) % alignment;

		return alignedOffset;
	}
}
