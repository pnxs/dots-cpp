#include <dots/type/NewVectorDescriptor.h>

namespace dots::type
{
	NewVectorDescriptor::NewVectorDescriptor(std::string name, const NewDescriptor<NewTypeless>& valueDescriptor, size_t size, size_t alignment):
		NewDescriptor<NewTypeless>(NewType::Vector, std::move(name), size, alignment),
		m_valueDescriptor(&valueDescriptor)
	{
		/* do nothing */
	}

	const NewDescriptor<NewTypeless>& NewVectorDescriptor::valueDescriptor() const
	{
		return *m_valueDescriptor;
	}
}
