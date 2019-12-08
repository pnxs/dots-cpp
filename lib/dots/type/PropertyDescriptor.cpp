#include <dots/type/PropertyDescriptor.h>
#include <dots/type/Struct.h>

namespace dots::type
{
	PropertyDescriptor<Typeless, void>::PropertyDescriptor(const std::shared_ptr<Descriptor<>>& descriptor, PropertyMetadata<> metadata) :
		m_descriptor{ descriptor },
		m_metadata{ std::move(metadata) }
	{
		m_name = m_metadata.name();
	}

	PropertyDescriptor<Typeless, void>::PropertyDescriptor(const std::shared_ptr<Descriptor<>>& descriptor, std::string name, size_t offset, uint32_t tag, bool isKey) :
		m_descriptor{ descriptor },
		m_name{ std::move(name) },
		m_metadata{ m_name, tag, isKey, descriptor->size(), descriptor->alignment(), offset }
	{
		/* do nothing */
	}

	PropertyDescriptor<Typeless, void>::PropertyDescriptor(const std::shared_ptr<Descriptor<>>& descriptor, std::string name, uint32_t tag, bool isKey) :
		m_descriptor{ descriptor },
		m_name{ std::move(name) },
		m_metadata{ m_name, tag, isKey, descriptor->size(), descriptor->alignment() }
	{
		/* do nothing */
	}

	PropertyDescriptor<Typeless, void>::PropertyDescriptor(const std::shared_ptr<Descriptor<>>& descriptor, std::string name, const PropertyDescriptor<>& previous, uint32_t tag, bool isKey) :
		m_descriptor{ descriptor },
		m_name{ std::move(name) },
		m_metadata{ m_name, tag, isKey, descriptor->size(), descriptor->alignment(), previous.metadata() }
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

	const PropertyMetadata<>& PropertyDescriptor<Typeless, void>::metadata() const
	{
		return m_metadata;
	}

	const std::string& PropertyDescriptor<Typeless, void>::name() const
	{
		return m_name;
	}

	size_t PropertyDescriptor<Typeless, void>::offset() const
	{
		return m_metadata.offset();
	}

	uint32_t PropertyDescriptor<Typeless, void>::tag() const
	{
		return m_metadata.tag();
	}

	bool PropertyDescriptor<Typeless, void>::isKey() const
	{
		return m_metadata.isKey();
	}

	PropertySet PropertyDescriptor<Typeless, void>::set() const
	{
		return m_metadata.set();
	}

	char* PropertyDescriptor<Typeless, void>::address(void* p) const
	{
		return reinterpret_cast<char*>(&reinterpret_cast<Struct*>(p)->_propertyArea()) + offset();
	}

	const char* PropertyDescriptor<Typeless, void>::address(const void* p) const
	{
		return reinterpret_cast<const char*>(&reinterpret_cast<const Struct*>(p)->_propertyArea()) + offset();
	}
}