#pragma once
#include <dots/type/PropertyArea.h>
#include <dots/type/Typeless.h>

namespace dots::type
{
	template <typename T = Typeless, typename = void>
	struct PropertyOffset;

	template <>
	struct PropertyOffset<>
	{
		constexpr PropertyOffset(std::in_place_t, size_t offset) :
			m_offset(offset)
		{
			/* do nothing */
		}
		
		constexpr PropertyOffset(size_t alignment) :
			PropertyOffset(alignment, 0, sizeof(PropertyArea))
		{
			/* do nothing */
		}

		constexpr PropertyOffset(size_t alignment, size_t previousOffset, size_t previousSize) :
			m_offset(CalculateOffset(alignment, previousOffset, previousSize))
		{
			/* do nothing */
		}
		
		PropertyOffset(const PropertyOffset& other) = default;
		PropertyOffset(PropertyOffset&& other) = default;
		~PropertyOffset() = default;

		PropertyOffset& operator = (const PropertyOffset& rhs) = default;
		PropertyOffset& operator = (PropertyOffset&& rhs) = default;

		constexpr operator size_t() const
		{
		    return m_offset;
		}

		constexpr size_t offset() const
		{
			return m_offset;
		}

	private:

		static constexpr size_t CalculateOffset(size_t alignment, size_t previousOffset, size_t previousSize)
		{
			size_t currentOffset = previousOffset + previousSize;
			size_t alignedOffset = currentOffset + (alignment - currentOffset % alignment) % alignment;

			return alignedOffset;
		}

		size_t m_offset;
	};

	template <typename T>
	struct PropertyOffset<T> : PropertyOffset<>
	{
		static constexpr PropertyOffset<T> Make()
		{
			return PropertyOffset<T>{};
		}

		template <typename U>
		static constexpr PropertyOffset<T> Make(const PropertyOffset<U>& previous)
		{
			return PropertyOffset<T>{ previous, sizeof(U) };
		}
		
		PropertyOffset(const PropertyOffset& other) = default;
		PropertyOffset(PropertyOffset&& other) = default;
		~PropertyOffset() = default;

		PropertyOffset& operator = (const PropertyOffset& rhs) = default;
		PropertyOffset& operator = (PropertyOffset&& rhs) = default;

	private:

		constexpr PropertyOffset() :
			PropertyOffset<>(alignof(T))
		{
			/* do nothing */
		}

		constexpr PropertyOffset(size_t previousOffset, size_t previousSize) :
			PropertyOffset<>(alignof(T), previousOffset, previousSize)
		{
			/* do nothing */
		}
	};
}