#include <dots/type/NewEnumDescriptor.h>
#include <dots/io/NewDescriptorConverter.h>

namespace dots::type
{
	NewEnumeratorDescriptor<NewTypeless>::NewEnumeratorDescriptor(uint32_t tag, std::string name) :
		m_tag(tag),
		m_name(std::move(name))
	{
		/* do nothing */
	}

	uint32_t NewEnumeratorDescriptor<NewTypeless>::tag() const
	{
		return m_tag;
	}

	const std::string& NewEnumeratorDescriptor<NewTypeless>::name() const
	{
		return m_name;
	}

	NewEnumDescriptor<NewTypeless, void>::NewEnumDescriptor(std::string name, const NewDescriptor<NewTypeless>& underlyingDescriptor):
		NewDescriptor<NewTypeless>(NewType::Enum, std::move(name), underlyingDescriptor.size(), underlyingDescriptor.alignment())
	{
		/* do nothing */
	}

	const types::EnumDescriptorData& NewEnumDescriptor<NewTypeless, void>::descriptorData() const
	{
		if (m_descriptorData == nullptr)
		{
			m_descriptorData = new types::EnumDescriptorData{ io::NewDescriptorConverter{}(*this) };
		}
		
		return *m_descriptorData;
	}
	
	const NewEnumDescriptor<>* NewEnumDescriptor<NewTypeless, void>::createFromEnumDescriptorData(const types::EnumDescriptorData& sd)
	{
		return io::NewDescriptorConverter{}(sd).get();
	}
}