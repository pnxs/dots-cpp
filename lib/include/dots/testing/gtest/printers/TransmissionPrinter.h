#pragma once
#include <iostream>
#include <dots/io/Transmission.h>
#include <dots/serialization/StringSerializer.h>

namespace dots::io
{
    /*!
     * @brief Print a DOTS transmission to an output stream.
     *
     * @remark This function is intended to be used indirectly by the
     * Google Test printing system.
     *
     * @remark The printer is using the global dots::to_string() function
     * to perform the string conversion of the transmission instance.
     *
     * @param transmission The DOTS transmission to print.
     *
     * @param os The output stream to print to.
     */
    inline void PrintTo(const Transmission& transmission, std::ostream* os)
    {
        if (transmission.header().isFromMyself == true)
        {
            *os << (transmission.header().removeObj == true ? "SELF REMOVE        " : "SELF CREATE/UPDATE ");
        }
        else
        {
            *os << (transmission.header().removeObj == true ? "     REMOVE        " : "     CREATE/UPDATE ");
        }

        *os << to_string(*transmission.instance(), transmission.instance()->_validProperties());
    }
}