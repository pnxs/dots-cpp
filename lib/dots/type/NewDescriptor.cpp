#include "NewDescriptor.h"

namespace dots::type
{
	NewDescriptor<NewTypeless>::NewDescriptor(NewType type, std::string name, size_t size, size_t alignment):
		m_type(type),
		m_name(std::move(name)),
		m_size(size),
		m_alignment(alignment)
	{
		/* do nothing */
	}

	NewType NewDescriptor<NewTypeless>::type() const
	{
		return m_type;
	}

	const std::string& NewDescriptor<NewTypeless>::name() const
	{
		return m_name;
	}

	size_t NewDescriptor<NewTypeless>::size() const
	{
		return m_size;
	}

	size_t NewDescriptor<NewTypeless>::alignment() const
	{
		return m_alignment;
	}

	bool NewDescriptor<NewTypeless>::usesDynamicMemory(const NewTypeless&/* value*/) const
	{
		return false;
	}

	size_t NewDescriptor<NewTypeless>::dynamicMemoryUsage(const NewTypeless&/* value*/) const
	{
		return 0;
	}
}