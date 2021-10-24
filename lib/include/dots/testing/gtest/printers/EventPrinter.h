#pragma once
#include <iostream>
#include <dots/Event.h>
#include <dots/serialization/StringSerializer.h>

namespace dots
{
    /*!
     * @brief Print a DOTS event to an output stream.
     *
     * @remark This function is intended to be used indirectly by the
     * Google Test printing system.
     *
     * @remark The printer is using the global dots::to_string() function
     * to perform the string conversion of the transmitted event instance.
     *
     * @tparam T The struct type of the DOTS event.
     *
     * @param event The DOTS event to print.
     *
     * @param os The output stream to print to.
     */
    template <typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
    void PrintTo(const Event<T>& event, std::ostream* os)
    {
        if (event.header().isFromMyself == true)
        {
            *os << (event.header().removeObj == true ? "SELF REMOVE        " : "SELF CREATE/UPDATE ");
        }
        else
        {
            *os << (event.header().removeObj == true ? "     REMOVE        " : "     CREATE/UPDATE ");
        }

        *os << to_string(event.transmitted(), event.transmitted()._validProperties());
    }
}