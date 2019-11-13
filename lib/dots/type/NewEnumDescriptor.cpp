#include <dots/type/NewEnumDescriptor.h>

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
}
