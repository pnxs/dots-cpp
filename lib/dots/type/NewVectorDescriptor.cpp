#include <dots/type/NewVectorDescriptor.h>

namespace dots::type
{
	NewVectorDescriptor::NewVectorDescriptor(std::string name, const std::shared_ptr<NewDescriptor<>>& valueDescriptor, size_t size, size_t alignment):
		NewDescriptor<NewTypeless>(NewType::Vector, std::move(name), size, alignment),
		m_valueDescriptor(valueDescriptor)
	{
		/* do nothing */
	}

	const std::shared_ptr<NewDescriptor<>>& NewVectorDescriptor::valueDescriptorPtr() const
	{
		return m_valueDescriptor;
	}

	const NewDescriptor<NewTypeless>& NewVectorDescriptor::valueDescriptor() const
	{
		return *m_valueDescriptor;
	}
}
