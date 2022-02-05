#pragma once
#include <utility>

namespace dots::type
{
    struct PropertyOffset
    {
        constexpr PropertyOffset(std::in_place_t, size_t offset) :
            m_offset(offset)
        {
            /* do nothing */
        }

        constexpr operator size_t() const
        {
            return m_offset;
        }

        constexpr size_t offset() const
        {
            return m_offset;
        }

        static constexpr PropertyOffset First(size_t alignment, size_t initialOffset)
        {
            return PropertyOffset{ alignment, 0, initialOffset };
        }

        static constexpr PropertyOffset Next(size_t alignment, size_t previousOffset, size_t previousSize)
        {
            return PropertyOffset{ alignment, previousOffset, previousSize  }; 
        }

    protected:

        constexpr PropertyOffset(size_t alignment, size_t previousOffset, size_t previousSize) :
            m_offset(CalculateOffset(alignment, previousOffset, previousSize))
        {
            /* do nothing */
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
}
