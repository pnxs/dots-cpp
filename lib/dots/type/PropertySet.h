#pragma once
#include <cstdint>
#include <bitset>
#include <limits>

namespace dots::type
{
	struct PropertySet
	{
		using value_t = uint32_t;
		using index_t = uint32_t;

		static_assert(!std::numeric_limits<value_t>::is_signed);
		static_assert(!std::numeric_limits<index_t>::is_signed);
		
		static_assert(std::numeric_limits<value_t>::digits == 32);
		static_assert(std::numeric_limits<index_t>::max() >= std::numeric_limits<value_t>::digits);		

		static constexpr index_t MaxProperties = std::numeric_limits<value_t>::digits;
		static constexpr index_t FirstProperty = 0;
		static constexpr index_t LastProperty = MaxProperties - 1;
		
		struct all_t{};
		struct none_t{};
		
		static constexpr none_t None{};
		static constexpr all_t All{};

		explicit constexpr PropertySet(value_t value) :
			m_value(value)
		{
			/* do nothing */
		}

		constexpr PropertySet() : PropertySet(0u)
		{
			/* do nothing */
		}

		constexpr PropertySet(none_t) : PropertySet(0u)
		{
			/* do nothing */
		}

		constexpr PropertySet(all_t) : PropertySet(~0u)
		{
			/* do nothing */
		}

		constexpr PropertySet(const PropertySet& other) = default;
		constexpr PropertySet(PropertySet&& other) noexcept = default;
		~PropertySet() = default;

		constexpr PropertySet& operator = (const PropertySet& rhs) = default;
		constexpr PropertySet& operator = (PropertySet&& rhs) noexcept = default;

		constexpr operator bool () const
		{
			return !empty();
		}

		constexpr bool operator == (const PropertySet& rhs) const
	    {
	        return m_value == rhs.m_value;
	    }
	    
	    constexpr bool operator != (const PropertySet& rhs) const
	    {
	        return !(*this == rhs);
	    }

		constexpr bool operator < (const PropertySet& rhs) const
	    {
	        return m_value != rhs.m_value && *this <= rhs;
	    }

		constexpr bool operator > (const PropertySet& rhs) const
	    {
	        return rhs < *this;
	    }

		constexpr bool operator <= (const PropertySet& rhs) const
	    {
	        return (m_value & rhs.m_value) == m_value;
	    }

		constexpr bool operator >= (const PropertySet& rhs) const
	    {
	        return rhs <= *this;
	    }

		constexpr PropertySet operator + (const PropertySet& rhs) const
		{
			PropertySet result = *this;
			result += rhs;

			return result;
		}

		constexpr PropertySet operator - (const PropertySet& rhs) const
		{
			PropertySet result = *this;
			result -= rhs;

			return result;
		}

		constexpr PropertySet operator ^ (const PropertySet& rhs) const
		{
			PropertySet result = *this;
			result ^= rhs;
			
			return result;
		}

		constexpr PropertySet operator ~ () const
	    {
			return PropertySet{ ~m_value };
	    }

		constexpr PropertySet& operator += (const PropertySet& rhs)
		{
			m_value |= rhs.m_value;
			return *this;
		}

		constexpr PropertySet& operator -= (const PropertySet& rhs)
		{
			m_value &= ~rhs.m_value;
			return *this;
		}

		constexpr PropertySet& operator ^= (const PropertySet& rhs)
		{
			m_value &= rhs.m_value;
			return *this;
		}

		constexpr PropertySet intersection(const PropertySet& rhs) const
		{
			PropertySet result = *this;
			return result.intersect(rhs);
		}

		constexpr PropertySet& intersect(const PropertySet& rhs)
		{
			m_value &= rhs.m_value;
			return *this;
		}

		constexpr PropertySet symmetricDifference(const PropertySet& rhs) const
		{
			PropertySet result = *this;
			return result.symmetricSubtract(rhs);
		}

		constexpr PropertySet& symmetricSubtract(const PropertySet& rhs)
		{
			m_value ^= rhs.m_value;
			return *this;
		}

		constexpr bool empty() const
		{
			return m_value == 0u;
		}

		constexpr index_t firstValid(index_t start = FirstProperty) const
		{
			// note: this is a slightly modified version of the DeBruijn algorithm to find the number of consecutive
			// 
			// clear all bits before start position
			value_t value = m_value >> start;
			value <<= start;
			
			// zeroes in an integer
			// source: http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightMultLookup
			constexpr int DeBruijnBitMultiplyToPosition[32] = 
			{
				0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
				31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
			};			
			
			return DeBruijnBitMultiplyToPosition[static_cast<index_t>((value & -value) * 0x077CB531u) >> 27];
		}

		constexpr index_t lastValid(index_t start = LastProperty) const
		{
			// note: this is a slightly modified version of the DeBruijn algorithm
			// 
			// clear all bits after start position
			index_t distance = LastProperty - start;
			value_t value = m_value << distance;
			value >>= distance;

			// find most significant bit
			// source: http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn
			constexpr int DeBruijnBitMultiplyToPosition[32] = 
			{
				0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
				8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
			};			
			
			value |= value >> 1;
			value |= value >> 2;
			value |= value >> 4;
			value |= value >> 8;
			value |= value >> 16;

			return DeBruijnBitMultiplyToPosition[static_cast<index_t>(value * 0x07C4ACDDu) >> 27];
		}

		size_t count() const
		{
			return toBitSet().count();
		}

		std::string toString(char zero = '0', char one = '1') const
		{
			return toBitSet().to_string(zero, one);
		}

		constexpr value_t toValue() const
		{
			return m_value;
		}

		static constexpr PropertySet FromIndex(index_t index)
		{
			return PropertySet{ 0x1u << index };
		}

		[[deprecated("only available for backwards compatibility")]]
		std::string to_string() const
		{
			return toString();
		}

	private:

		using set_t = std::bitset<8 * sizeof(value_t)>;

		constexpr set_t toBitSet() const
		{
			return set_t{ m_value };
		}

		value_t m_value;
	};
}