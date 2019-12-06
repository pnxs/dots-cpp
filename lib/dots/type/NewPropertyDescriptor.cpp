#include <dots/type/NewPropertyDescriptor.h>
#include <dots/type/NewPropertyArea.h>
#include <dots/type/NewStruct.h>

namespace dots::type
{
	NewPropertyDescriptor<NewTypeless, void>::NewPropertyDescriptor(const std::shared_ptr<NewDescriptor<>>& descriptor, std::string name, size_t offset, uint32_t tag, bool isKey):
		m_descriptor(descriptor),
		m_name(std::move(name)),
		m_offset(offset),
		m_tag(tag),
		m_isKey(isKey),
		m_set{NewPropertySet::FromIndex(m_tag)}
	{
		/* do nothing */
	}

	NewPropertyDescriptor<NewTypeless, void>::NewPropertyDescriptor(const std::shared_ptr<NewDescriptor<>>& descriptor, std::string name, uint32_t tag, bool isKey):
		NewPropertyDescriptor(descriptor, std::move(name), CalculateOffset(*descriptor, 0, sizeof(NewPropertyArea)), tag, isKey)
	{
		/* do nothing */
	}

	NewPropertyDescriptor<NewTypeless, void>::NewPropertyDescriptor(const std::shared_ptr<NewDescriptor<>>& descriptor, std::string name, const NewPropertyDescriptor<>& previous, uint32_t tag, bool isKey):
		NewPropertyDescriptor(descriptor, std::move(name), CalculateOffset(*descriptor, previous), tag, isKey)
	{
		/* do nothing */
	}

	const std::shared_ptr<NewDescriptor<>>& NewPropertyDescriptor<NewTypeless, void>::valueDescriptorPtr() const
	{
		return m_descriptor;
	}

	const NewDescriptor<>& NewPropertyDescriptor<NewTypeless, void>::valueDescriptor() const
	{
		return *m_descriptor;
	}

	const std::string& NewPropertyDescriptor<NewTypeless, void>::name() const
	{
		return m_name;
	}

	size_t NewPropertyDescriptor<NewTypeless, void>::offset() const
	{
		return m_offset;
	}

	uint32_t NewPropertyDescriptor<NewTypeless, void>::tag() const
	{
		return m_tag;
	}

	bool NewPropertyDescriptor<NewTypeless, void>::isKey() const
	{
		return m_isKey;
	}

	NewPropertySet NewPropertyDescriptor<NewTypeless, void>::set() const
	{
		return m_set;
	}

	size_t NewPropertyDescriptor<NewTypeless, void>::CalculateOffset(const NewDescriptor<>& descriptor, const NewPropertyDescriptor<>& previous)
	{
		return CalculateOffset(descriptor, previous.offset(), previous.valueDescriptor().size());
	}

	size_t NewPropertyDescriptor<NewTypeless, void>::CalculateOffset(const NewDescriptor<>& descriptor, size_t previousOffset, size_t previousSize)
	{
		size_t currentOffset = previousOffset + previousSize;
		size_t alignment = descriptor.alignment();
		size_t alignedOffset = currentOffset + (alignment - currentOffset % alignment) % alignment;

		return alignedOffset;
	}
}
