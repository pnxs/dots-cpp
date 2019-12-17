#include <dots/type/VectorDescriptor.h>

namespace dots::type
{
	VectorDescriptor::VectorDescriptor(std::string name, const std::shared_ptr<Descriptor<>>& valueDescriptor, size_t size, size_t alignment):
		Descriptor<Typeless>(Type::Vector, std::move(name), size, alignment),
		m_valueDescriptor(valueDescriptor)
	{
		/* do nothing */
	}

	const std::shared_ptr<Descriptor<>>& VectorDescriptor::valueDescriptorPtr() const
	{
		return m_valueDescriptor;
	}

	const Descriptor<Typeless>& VectorDescriptor::valueDescriptor() const
	{
		return *m_valueDescriptor;
	}
}
