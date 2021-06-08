#pragma once
#include <dots/testing/gtest/printers/EventPrinter.h>
#include <dots/testing/gtest/matchers/StructMatcher.h>

namespace dots::testing
{
    template <typename T>
    struct EventEqualMatcher
    {
        using is_gtest_matcher = void;

        EventEqualMatcher(T instance, property_set_t includedProperties, bool remove) :
            m_expectedStructMatcher{ std::move(instance), includedProperties },
            m_remove(remove)
        {
            /* do nothing */
        }
        EventEqualMatcher(const EventEqualMatcher& other) = default;
        EventEqualMatcher(EventEqualMatcher&& other) = default;
        ~EventEqualMatcher() = default;

        EventEqualMatcher& operator = (const EventEqualMatcher& rhs) = default;
        EventEqualMatcher& operator = (EventEqualMatcher&& rhs) = default;

        bool MatchAndExplain(const Event<>& e, std::ostream* os) const
        {
            const DotsHeader& header = e.header();
            const type::Struct& instance = e.transmitted();

            if (bool isRemove = header.removeObj == true; isRemove != m_remove)
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
            *os << (m_remove ? "REMOVE        " : "CREATE/UPDATE ");
            m_expectedStructMatcher.DescribeTo(os);
        }

        void DescribeNegationTo(std::ostream* os) const
        {
            *os << (m_remove ? "REMOVE        " : "CREATE/UPDATE ");
            m_expectedStructMatcher.DescribeNegationTo(os);
        }

    private:

        StructEqualMatcher<T> m_expectedStructMatcher;
        bool m_remove;
    };

    template <typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
    ::testing::Matcher<const Event<>&> EventEqual(T instance, std::optional<types::property_set_t> includedProperties = std::nullopt, bool remove = false)
    {
        types::property_set_t includedProperties_ = includedProperties == std::nullopt ? instance._validProperties() : *includedProperties;
        return EventEqualMatcher<T>(std::move(instance), includedProperties_, remove);
    }
}