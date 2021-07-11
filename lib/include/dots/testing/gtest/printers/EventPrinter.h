#pragma once
#include <iostream>
#include <dots/Event.h>
#include <dots/serialization/StringSerializer.h>

namespace dots
{
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