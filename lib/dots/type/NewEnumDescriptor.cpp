#include <dots/type/NewEnumDescriptor.h>
#include <dots/io/NewDescriptorConverter.h>

namespace dots::type
{
	EnumeratorDescriptor<Typeless>::EnumeratorDescriptor(uint32_t tag, std::string name) :
		m_tag(tag),
		m_name(std::move(name))
	{
		/* do nothing */
	}

	uint32_t EnumeratorDescriptor<Typeless>::tag() const
	{
		return m_tag;
	}

	const std::string& EnumeratorDescriptor<Typeless>::name() const
	{
		return m_name;
	}

	EnumDescriptor<Typeless, void>::EnumDescriptor(std::string name, const Descriptor<Typeless>& underlyingDescriptor):
		Descriptor<Typeless>(Type::Enum, std::move(name), underlyingDescriptor.size(), underlyingDescriptor.alignment())
	{
		/* do nothing */
	}

	const types::EnumDescriptorData& EnumDescriptor<Typeless, void>::descriptorData() const
	{
		if (m_descriptorData == nullptr)
		{
			m_descriptorData = new types::EnumDescriptorData{ io::DescriptorConverter{}(*this) };
		}
		
		return *m_descriptorData;
	}
	
	const EnumDescriptor<>* EnumDescriptor<Typeless, void>::createFromEnumDescriptorData(const types::EnumDescriptorData& sd)
	{
		return io::DescriptorConverter{}(sd).get();
	}
}