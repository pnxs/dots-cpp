#include "Enum.h"
#include <algorithm>
#include "EnumElementDescriptor.dots.h"

namespace dots::type
{
	Enum<void>::Enum(const EnumDescriptor& descriptor, int32_t value) :
		_descriptor(&descriptor),
		_value(value)
	{
		/* do nothing */
	}

	const EnumDescriptor& Enum<void>::descriptor() const
	{
		return *_descriptor;
	}

	int32_t Enum<void>::value() const
	{
		return _value;
	}

	uint32_t Enum<void>::tag() const
	{
		for (const auto& [tag, enumeratorDescriptor] : _descriptor->elements())
		{
			if (enumeratorDescriptor.enum_value == _value)
			{
				return tag;
			}
		}

		return 0;
	}

	const std::string& Enum<void>::identifier() const
	{
		return *std::find_if(_descriptor->elements().begin(), _descriptor->elements().end(), [&](const auto& kv)
		{
			const auto&[tag, enumeratorDescriptor] = kv;
			(void)tag;
			return enumeratorDescriptor.enum_value == _value;
		})->second.name;
	}
}