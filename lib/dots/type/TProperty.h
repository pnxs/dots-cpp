#pragma once
#include <string_view>
#include "Struct.h"
#include "StructDescriptor.h"

namespace dots::type
{
	class Descriptor;
	class StructProperty;

    template <typename T, typename Derived, typename Previous>
    struct TProperty
    {
		using value_t = T;

        TProperty() = default;
        TProperty(const TProperty& other) = default;
        TProperty(TProperty&& other) = default;
        ~TProperty() = default;

        TProperty& operator = (const TProperty& rhs) = default;
        TProperty& operator = (TProperty&& rhs) = default;

        static constexpr std::string_view Name()
        {
            return Derived::Description.name;
        }

        static constexpr size_t Offset()
        {
            return Derived::Description.offset;
        }

        static constexpr uint32_t Tag()
        {
            return Derived::Description.tag;
        }

        static constexpr bool IsKey()
        {
            return Derived::Description.isKey;
        }

    protected:

		static constexpr Struct::PropertyDescription MakePropertyDescription(uint32_t tag, const std::string_view& type, const std::string_view& name, bool isKey)
		{
			return Struct::PropertyDescription{ CalculateOffset(), tag, isKey, name, type };
		}

    private:

		static constexpr size_t CalculateOffset()
		{
			constexpr size_t currentOffset = []()
			{
				if constexpr (std::is_same_v<Previous, void>)
				{
					return sizeof(Struct);
				}
				else
				{
					return Previous::Offset() + sizeof(typename Previous::value_t);
				}
			}();

			constexpr size_t alignment = alignof(value_t);
			constexpr size_t alignedOffset = currentOffset + (alignment - currentOffset % alignment) % alignment;

			return alignedOffset;
		}

        value_t _value;
    };
}