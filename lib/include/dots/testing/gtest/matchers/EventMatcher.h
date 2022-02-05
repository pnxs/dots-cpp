#pragma once
#include <dots/testing/gtest/printers/EventPrinter.h>
#include <dots/testing/gtest/matchers/StructMatcher.h>

namespace dots::testing
{
    namespace details
    {
        template <typename T>
        struct EventEqualMatcher
        {
            using is_gtest_matcher = void;

            EventEqualMatcher(T instance, DotsHeader header) :
                m_expectedStructMatcher{ std::move(instance), header.attributes },
                m_header(std::move(header))
            {
                /* do nothing */
            }

            bool MatchAndExplain(const Event<>& event, std::ostream* os) const
            {
                const DotsHeader& header = event.header();
                const type::Struct& instance = event.transmitted();

                if ((m_header.removeObj.isValid() && header.removeObj != m_header.removeObj) || 
                    (m_header.isFromMyself.isValid() && header.isFromMyself != m_header.isFromMyself))
                {
                    return false;
                }
                else 
                {
                    return m_expectedStructMatcher.MatchAndExplain(instance, os);
                }
            }

            void DescribeTo(std::ostream* os) const
            {
                describeHeaderTo(os);
                m_expectedStructMatcher.DescribeTo(os);
            }

            void DescribeNegationTo(std::ostream* os) const
            {
                describeHeaderTo(os);
                m_expectedStructMatcher.DescribeNegationTo(os);
            }

        private:

            void describeHeaderTo(std::ostream* os) const
            {
                if (m_header.isFromMyself == true)
                {
                    *os << (m_header.removeObj == true ? "SELF REMOVE        " : "SELF CREATE/UPDATE ");
                }
                else
                {
                    *os << (m_header.removeObj == true ? "     REMOVE        " : "     CREATE/UPDATE ");
                }
            }

            StructEqualMatcher<T> m_expectedStructMatcher;
            DotsHeader m_header;
        };
    }

    /*!
     * @brief Create a Google Test matcher that compares DOTS events for
     * equality.
     *
     * An event will be matched as equal if the transmitted instance in the
     * event is equal to the given instance for the given property set.
     *
     * Additionally, the event must match the given @p remove and
     * @p isFromMyself flags.
     *
     * @tparam T The DOTS struct type of the event.
     *
     * @param instance The instance to compare the transmitted instance in
     * the event to.
     *
     * @param includedProperties The property set to include in the
     * equality comparison. If no set is given, the valid property set of
     * @p instance will be used.
     *
     * @param remove Specifies whether matched events must be remove
     * events.
     *
     * @param isFromMyself Specifies whether matched events must originate
     * from myself.
     *
     * @return ::testing::Matcher<const Event<>&> The created Google Test
     * matcher.
     */
    template <typename T>
    ::testing::Matcher<const Event<>&> EventEqual(T&& instance, std::optional<types::property_set_t> includedProperties = std::nullopt, bool remove = false, bool isFromMyself = false)
    {
        using decayed_t = std::decay_t<T>;
        constexpr bool IsStruct = std::is_base_of_v<type::Struct, decayed_t>;
        static_assert(IsStruct, "instance type T has to be a DOTS struct type");

        if constexpr (IsStruct)
        {
            DotsHeader header{
                DotsHeader::attributes_i{ includedProperties == std::nullopt ? instance._validProperties() : *includedProperties },
            };

            if (remove)
            {
                header.removeObj = true;
            }

            if (isFromMyself)
            {
                header.isFromMyself = true;
            }

            return details::EventEqualMatcher<decayed_t>(
                std::forward<T>(instance), 
                std::move(header)
            );
        }
        else
        {
            return std::declval<::testing::Matcher<const Event<>&>>();
        }
    }
}
