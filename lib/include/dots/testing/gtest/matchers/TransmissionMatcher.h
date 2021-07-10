#pragma once
#include <type_traits>
#include <dots/testing/gtest/printers/TransmissionPrinter.h>
#include <dots/testing/gtest/matchers/StructMatcher.h>

namespace dots::testing
{
    template <typename T>
    struct TransmissionEqualMatcher
    {
        using is_gtest_matcher = void;

        TransmissionEqualMatcher(T instance, property_set_t includedProperties, bool remove) :
            m_expectedStructMatcher{ std::move(instance), includedProperties },
            m_remove(remove)
        {
            /* do nothing */
        }
        TransmissionEqualMatcher(const TransmissionEqualMatcher& other) = default;
        TransmissionEqualMatcher(TransmissionEqualMatcher&& other) = default;
        ~TransmissionEqualMatcher() = default;

        TransmissionEqualMatcher& operator = (const TransmissionEqualMatcher& rhs) = default;
        TransmissionEqualMatcher& operator = (TransmissionEqualMatcher&& rhs) = default;

        bool MatchAndExplain(const io::Transmission& transmission, std::ostream* os) const
        {
            const DotsHeader& header = transmission.header();
            const type::Struct& instance = transmission.instance();

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

    template <typename T>
    ::testing::Matcher<const io::Transmission&> TransmissionEqual(T&& instance, std::optional<types::property_set_t> includedProperties = std::nullopt, bool remove = false)
    {
        using decayed_t = std::decay_t<T>;
        constexpr bool IsStruct = std::is_base_of_v<dots::type::Struct, decayed_t>;
        static_assert(IsStruct, "instance type T has to be a DOTS struct type");

        if constexpr (IsStruct)
        {
            types::property_set_t includedProperties_ = includedProperties == std::nullopt ? instance._validProperties() : *includedProperties;
            return TransmissionEqualMatcher<decayed_t>(std::forward<T>(instance), includedProperties_, remove);
        }
        else
        {
            return std::declval<::testing::Matcher<const io::Transmission&>>();
        }
    }
}