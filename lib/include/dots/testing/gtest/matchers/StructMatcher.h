#pragma once
#include <type_traits>
#include <dots/serialization/StringSerializer.h>

namespace dots::testing
{
    template <typename Expected>
    struct StructEqualMatcher
    {
        using is_gtest_matcher = void;

        StructEqualMatcher(Expected expected, const property_set_t& includedProperties = property_set_t::All) :
            m_expected{ std::move(expected) },
            m_includedProperties(includedProperties)
        {
            /* do nothing */
        }
        StructEqualMatcher(const StructEqualMatcher& other) = default;
        StructEqualMatcher(StructEqualMatcher&& other) = default;
        ~StructEqualMatcher() = default;

        StructEqualMatcher& operator = (const StructEqualMatcher& rhs) = default;
        StructEqualMatcher& operator = (StructEqualMatcher&& rhs) = default;

        template <typename Actual>
        bool MatchAndExplain(const Actual& actual, std::ostream* os) const
        {
            if (&actual._descriptor() != &m_expected._descriptor())
            {
                if (os != nullptr)
                {
                    *os << "instances are not of the same type";
                }

                return false;
            }

            if (actual._equal(m_expected, m_includedProperties))
            {
                return true;
            }
            else
            {
                property_set_t diff = actual._diffProperties(m_expected, m_includedProperties);

                if (os != nullptr)
                {
                    *os << "\n" << 
                        "             Diff: " << to_string(m_expected, diff) << "\n" <<
                        "                 : " << to_string(actual, diff);
                }

                return false;
            }
        }

        void DescribeTo(std::ostream* os) const
        {
            *os << to_string(m_expected, m_includedProperties);
        }

        void DescribeNegationTo(std::ostream* os) const
        {
            *os << to_string(m_expected, ~m_includedProperties);
        }

    private:

        Expected m_expected;
        property_set_t m_includedProperties;
    };

    template <typename Expected, std::enable_if_t<std::is_base_of_v<type::Struct, std::decay_t<Expected>>, int> = 0>
    ::testing::Matcher<const type::Struct&> StructEqual(Expected&& expected, const property_set_t& includedProperties = property_set_t::All)
    {
        return StructEqualMatcher<std::decay_t<Expected>>(std::forward<Expected>(expected), includedProperties);
    }

    template <typename Expected, std::enable_if_t<std::is_base_of_v<type::Struct, std::decay_t<Expected>>, int> = 0>
    ::testing::Matcher<const std::decay_t<Expected>&> StaticStructEqual(Expected&& expected, const property_set_t& includedProperties = property_set_t::All)
    {
        return StructEqualMatcher<std::decay_t<Expected>>(std::forward<Expected>(expected), includedProperties);
    }
}