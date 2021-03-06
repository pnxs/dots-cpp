#pragma once
#include <iostream>
#include <dots/Event.h>
#include <dots/serialization/StringSerializer.h>

namespace dots::io
{
    template <typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
    void PrintTo(const Event<T>& e, std::ostream* os)
    {
        *os << (e.isRemove() ? "REMOVE        " : "CREATE/UPDATE ") << to_string(e.transmitted(), e.transmitted()._validProperties());
    }
}