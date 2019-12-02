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

	bool NewDescriptor<NewTypeless>::isFundamentalType() const
	{
		return IsFundamentalType(type());
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

	bool NewDescriptor<NewTypeless>::usesDynamicMemory() const
	{
		return false;
	}

	size_t NewDescriptor<NewTypeless>::dynamicMemoryUsage(const NewTypeless&/* value*/) const
	{
		return 0;
	}

	void NewDescriptor<NewTypeless>::fromString(NewTypeless&/* storage*/, const std::string_view&/* value*/) const
	{
		throw std::logic_error{ "from string construction not available for type: " + name() };
	}

	std::string NewDescriptor<NewTypeless>::toString(const NewTypeless&/* value*/) const
	{
		throw std::logic_error{ "to string conversion not available for type: " + name() };
	}

	bool NewDescriptor<NewTypeless>::IsFundamentalType(const NewDescriptor& descriptor)
	{
		return IsFundamentalType(descriptor.type());
	}

	bool NewDescriptor<NewTypeless>::IsFundamentalType(NewType type)
	{
		switch (type)
	    {
	        case NewType::boolean:
	        case NewType::int8:
	        case NewType::int16:
	        case NewType::int32:
	        case NewType::int64:
	        case NewType::uint8:
	        case NewType::uint16:
	        case NewType::uint32:
	        case NewType::uint64:
	        case NewType::float32:
	        case NewType::float64:
	        case NewType::property_set:
	        case NewType::timepoint:
	        case NewType::steady_timepoint:
	        case NewType::duration:
	        case NewType::string:
	        case NewType::uuid:
	            return true;

	        case NewType::Vector:
	        case NewType::Enum:
	        case NewType::Struct:
	            return false;
	    }

		return false;
	}
}