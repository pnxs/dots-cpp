#pragma once
#include <dots/serialization/StringSerializer.h>

namespace dots::type
{
    namespace details
    {
        template <typename T>
        constexpr bool is_gtest_printable_v = std::disjunction_v<
            std::is_same<T, property_set_t>,
            std::is_same<T, uuid_t>,
            std::is_same<T, timepoint_t>,
            std::is_same<T, steady_timepoint_t>,
            std::is_same<T, duration_t>,
            is_property<T>,
            is_enum<T>,
            std::is_base_of<Struct, T>
        >;
    }

    /*!
     * @brief Print a DOTS type value to an output stream.
     *
     * @remark This function is intended to be used indirectly by the
     * Google Test printing system.
     *
     * @remark The printer is using the global dots::to_string() functions
     * to perform the string conversion.
     *
     * @remark The printer will not be used for certain fundamental types
     * (e.g. integers and floats), which will be printed using the default
     * Google Test printer instead.
     *
     * @tparam T The DOTS type to print.
     *
     * @param os The output stream to print to.
     *
     * @param value The value to print.
     *
     * @return std::ostream& The given output stream @p os.
     */
    template <typename T, std::enable_if_t<details::is_gtest_printable_v<std::decay_t<T>>, int> = 0>
    std::ostream& operator << (std::ostream& os, T&& value)
    {
        return os << to_string(std::forward<T>(value));
    }
}

namespace dots::types
{
    /*!
     * @brief Print a DOTS enum type value to an output stream.
     *
     * @remark This function is intended to be used indirectly by the
     * Google Test printing system.
     *
     * @remark The printer is using the global dots::to_string() functions
     * to perform the string conversion.
     *
     * @tparam T The DOTS enum type to print.
     *
     * @param os The output stream to print to.
     *
     * @param value The value to print.
     *
     * @return std::ostream& The given output stream @p os.
     */
    template <typename T, std::enable_if_t<type::is_enum_v<std::decay_t<T>>, int> = 0>
    std::ostream& operator << (std::ostream& os, T&& value)
    {
        return os << to_string(std::forward<T>(value));
    }
}