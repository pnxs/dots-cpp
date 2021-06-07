#pragma once
#include <iostream>
#include <dots/io/Transmission.h>
#include <dots/io/serialization/StringSerializer.h>

namespace dots::io
{
    inline void PrintTo(const Transmission& transmission, std::ostream* os)
    {
        *os << (transmission.header().removeObj == true ? "REMOVE        " : "CREATE/UPDATE ") << to_string(*transmission.instance(), transmission.instance()->_validProperties());
    }
}