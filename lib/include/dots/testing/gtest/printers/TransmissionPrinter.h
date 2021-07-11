#pragma once
#include <iostream>
#include <dots/io/Transmission.h>
#include <dots/serialization/StringSerializer.h>

namespace dots::io
{
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