#pragma once
#include <type_traits>
#include <dots/serialization/StringSerializer.h>

namespace dots::testing
{
    namespace details
    {
        template <typename Expected>
        struct StructEqualMatcher
        {
            using is_gtest_matcher = void;

            StructEqualMatcher(Expected expected, property_set_t includedProperties = property_set_t::All) :
                m_expected{ std::move(expected) },
                m_includedProperties(includedProperties)
            {
                /* do nothing */
            }

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
    }

    /*!
     * @brief Create a Google Test matcher that compares DOTS struct
     * instances for equality.
     *
     * A struct will be matched as equal if the instance is equal to the
     * given instance for the given property set.
     *
     * @remark This matcher is comparing against instances of the Struct
     * base type. For an explicitly typed version see StaticStructEqual().
     *
     * @tparam Expected The DOTS struct type.
     *
     * @param expected The instance to match.
     *
     * @param includedProperties The property set to include in the
     * equality comparison.
     *
     * @return ::testing::Matcher<const type::Struct&> The created Google
     * Test matcher.
     */
    template <typename Expected, std::enable_if_t<std::is_base_of_v<type::Struct, std::decay_t<Expected>>, int> = 0>
    ::testing::Matcher<const type::Struct&> StructEqual(Expected&& expected, property_set_t includedProperties = property_set_t::All)
    {
        return details::StructEqualMatcher<std::decay_t<Expected>>(std::forward<Expected>(expected), includedProperties);
    }

    /*!
     * @brief Create a Google Test matcher that compares DOTS struct
     * instances for equality.
     *
     * A struct will be matched as equal if the instance in the
     * transmission is equal to the given instance for the given property
     * set.
     *
     * @remark This matcher is comparing against instances of the explicit
     * struct type. For Struct base class version see StructEqual().
     *
     * @tparam Expected The DOTS struct type.
     *
     * @param expected The instance to match.
     *
     * @param includedProperties The property set to include in the
     * equality comparison.
     *
     * @return ::testing::Matcher<const std::decay_t<Expected>&> The
     * created Google Test matcher.
     */
    template <typename Expected, std::enable_if_t<std::is_base_of_v<type::Struct, std::decay_t<Expected>>, int> = 0>
    ::testing::Matcher<const std::decay_t<Expected>&> StaticStructEqual(Expected&& expected, property_set_t includedProperties = property_set_t::All)
    {
        return details::StructEqualMatcher<std::decay_t<Expected>>(std::forward<Expected>(expected), includedProperties);
    }
}
