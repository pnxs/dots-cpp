#include <dots/type/Descriptor.h>

namespace dots::type
{
	Descriptor<Typeless>::Descriptor(Type type, std::string name, size_t size, size_t alignment):
		m_type(type),
		m_name(std::move(name)),
		m_size(size),
		m_alignment(alignment)
	{
		/* do nothing */
	}

	Type Descriptor<Typeless>::type() const
	{
		return m_type;
	}

	bool Descriptor<Typeless>::isFundamentalType() const
	{
		return IsFundamentalType(type());
	}

	const std::string& Descriptor<Typeless>::name() const
	{
		return m_name;
	}

	size_t Descriptor<Typeless>::size() const
	{
		return m_size;
	}

	size_t Descriptor<Typeless>::alignment() const
	{
		return m_alignment;
	}

	bool Descriptor<Typeless>::usesDynamicMemory() const
	{
		return false;
	}

	size_t Descriptor<Typeless>::dynamicMemoryUsage(const Typeless&/* value*/) const
	{
		return 0;
	}

	void Descriptor<Typeless>::fromString(Typeless&/* storage*/, const std::string_view&/* value*/) const
	{
		throw std::logic_error{ "from string construction not available for type: " + name() };
	}

	std::string Descriptor<Typeless>::toString(const Typeless&/* value*/) const
	{
		throw std::logic_error{ "to string conversion not available for type: " + name() };
	}

	bool Descriptor<Typeless>::IsFundamentalType(const Descriptor& descriptor)
	{
		return IsFundamentalType(descriptor.type());
	}

	bool Descriptor<Typeless>::IsFundamentalType(Type type)
	{
		switch (type)
	    {
	        case Type::boolean:
	        case Type::int8:
	        case Type::int16:
	        case Type::int32:
	        case Type::int64:
	        case Type::uint8:
	        case Type::uint16:
	        case Type::uint32:
	        case Type::uint64:
	        case Type::float32:
	        case Type::float64:
	        case Type::property_set:
	        case Type::timepoint:
	        case Type::steady_timepoint:
	        case Type::duration:
	        case Type::string:
	        case Type::uuid:
	            return true;

	        case Type::Vector:
	        case Type::Enum:
	        case Type::Struct:
	            return false;
	    }

		return false;
	}
}