#pragma once
#include <string>
#include <string_view>
#include <dots/type/PropertySet.h>
#include <dots/type/PropertyArea.h>

namespace dots::type
{
	template <typename T = Typeless, typename = void>
	struct PropertyMetadata;

	template <>
	struct PropertyMetadata<>
	{
		constexpr PropertyMetadata(const std::string_view& name, uint32_t tag, bool isKey, size_t size, size_t alignment, size_t offset) :
			m_name(name),
			m_tag(tag),
			m_isKey(isKey),
			m_set{PropertySet::FromIndex(m_tag)},
			m_size(size),
			m_alignment(alignment),
			m_offset(offset)
		{
			/* do nothing */
		}

		constexpr PropertyMetadata(const std::string_view& name, uint32_t tag, bool isKey, size_t size, size_t alignment) :
			PropertyMetadata(name, tag, isKey, size, alignment, CalculateOffset(alignment, 0, sizeof(PropertyArea)))
		{
			/* do nothing */
		}

		constexpr PropertyMetadata(const std::string_view& name, uint32_t tag, bool isKey, size_t size, size_t alignment, const PropertyMetadata<>& previous):
			PropertyMetadata(name, tag, isKey, size, alignment, CalculateOffset(alignment, previous.offset(), previous.size()))
		{
			/* do nothing */
		}
		
		PropertyMetadata(const PropertyMetadata& other) = default;
		PropertyMetadata(PropertyMetadata&& other) = default;
		~PropertyMetadata() = default;

		PropertyMetadata& operator = (const PropertyMetadata& rhs) = default;
		PropertyMetadata& operator = (PropertyMetadata&& rhs) = default;

		constexpr const std::string_view& name() const
		{
			return m_name;
		}

		constexpr uint32_t tag() const
		{
			return m_tag;
		}

		constexpr bool isKey() const
		{
			return m_isKey;
		}

		constexpr PropertySet set() const
		{
			return m_set;
		}

		constexpr size_t size() const
		{
			return m_size;
		}

		constexpr size_t alignment() const
		{
			return m_alignment;
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

		std::string_view m_name;
		uint32_t m_tag;
		bool m_isKey;
		PropertySet m_set;
		size_t m_size;
		size_t m_alignment;
		size_t m_offset;
	};

	template <typename T>
	struct PropertyMetadata<T> : PropertyMetadata<>
	{
		constexpr PropertyMetadata(const std::string_view& name, uint32_t tag, bool isKey, size_t offset) :
			PropertyMetadata<>(name, tag, isKey, sizeof(T), alignof(T), offset)
		{
			/* do nothing */
		}
		
		constexpr PropertyMetadata(const std::string_view& name, uint32_t tag, bool isKey) :
			PropertyMetadata<>(name, tag, isKey, sizeof(T), alignof(T))
		{
			/* do nothing */
		}
		
		constexpr PropertyMetadata(const std::string_view& name, uint32_t tag, bool isKey, const PropertyMetadata<>& previous) :
			PropertyMetadata<>(name, tag, isKey, sizeof(T), alignof(T), previous)
		{
			/* do nothing */
		}
		
		PropertyMetadata(const PropertyMetadata& other) = default;
		PropertyMetadata(PropertyMetadata&& other) = default;
		~PropertyMetadata() = default;

		PropertyMetadata& operator = (const PropertyMetadata& rhs) = default;
		PropertyMetadata& operator = (PropertyMetadata&& rhs) = default;
	};
}