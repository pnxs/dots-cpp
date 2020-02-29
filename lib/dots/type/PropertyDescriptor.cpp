#include <dots/type/PropertyDescriptor.h>
#include <dots/type/Struct.h>

namespace dots::type
{
	PropertyDescriptor::PropertyDescriptor(const std::shared_ptr<Descriptor<>>& descriptor, PropertyMetadata<> metadata) :
		m_descriptor{ descriptor },
		m_metadata{ std::move(metadata) }
	{
		m_name = m_metadata.name();
	}

	PropertyDescriptor::PropertyDescriptor(const std::shared_ptr<Descriptor<>>& descriptor, std::string name, size_t offset, uint32_t tag, bool isKey) :
		m_descriptor{ descriptor },
		m_name{ std::move(name) },
		m_metadata{ m_name, tag, isKey, descriptor->size(), descriptor->alignment(), offset }
	{
		/* do nothing */
	}

	PropertyDescriptor::PropertyDescriptor(const std::shared_ptr<Descriptor<>>& descriptor, std::string name, uint32_t tag, bool isKey) :
		m_descriptor{ descriptor },
		m_name{ std::move(name) },
		m_metadata{ m_name, tag, isKey, descriptor->size(), descriptor->alignment() }
	{
		/* do nothing */
	}

	PropertyDescriptor::PropertyDescriptor(const std::shared_ptr<Descriptor<>>& descriptor, std::string name, const PropertyDescriptor& previous, uint32_t tag, bool isKey) :
		m_descriptor{ descriptor },
		m_name{ std::move(name) },
		m_metadata{ m_name, tag, isKey, descriptor->size(), descriptor->alignment(), previous.metadata() }
	{
		/* do nothing */
	}

    PropertyDescriptor::PropertyDescriptor(const PropertyDescriptor& other) :
	    m_descriptor(other.m_descriptor),
	    m_name(other.name()),
	    m_metadata{ m_name, other.metadata().tag(), other.metadata().isKey(), other.metadata().size(), other.metadata().alignment(), other.metadata().offset() }
    {
		/* do nothing */
    }

    PropertyDescriptor& PropertyDescriptor::operator = (const PropertyDescriptor& rhs)
    {
		m_descriptor = rhs.m_descriptor;
	    m_name = rhs.name();
	    m_metadata =  PropertyMetadata<>{ m_name, rhs.metadata().tag(), rhs.metadata().isKey(), rhs.metadata().size(), rhs.metadata().alignment(), rhs.metadata().offset() };

		return *this;
    }

    const std::shared_ptr<Descriptor<>>& PropertyDescriptor::valueDescriptorPtr() const
	{
		return m_descriptor;
	}

	const Descriptor<>& PropertyDescriptor::valueDescriptor() const
	{
		return *m_descriptor;
	}

	const PropertyMetadata<>& PropertyDescriptor::metadata() const
	{
		return m_metadata;
	}

	const std::string& PropertyDescriptor::name() const
	{
		return m_name;
	}

	size_t PropertyDescriptor::offset() const
	{
		return m_metadata.offset();
	}

	uint32_t PropertyDescriptor::tag() const
	{
		return m_metadata.tag();
	}

	bool PropertyDescriptor::isKey() const
	{
		return m_metadata.isKey();
	}

	PropertySet PropertyDescriptor::set() const
	{
		return m_metadata.set();
	}

	char* PropertyDescriptor::address(void* p) const
	{
		return reinterpret_cast<char*>(&reinterpret_cast<Struct*>(p)->_propertyArea()) + offset();
	}

	const char* PropertyDescriptor::address(const void* p) const
	{
		return reinterpret_cast<const char*>(&reinterpret_cast<const Struct*>(p)->_propertyArea()) + offset();
	}
}