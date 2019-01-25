#include "UuidDescriptor.h"

template<>
std::string to_string<dots::uuid>(const dots::uuid& v)
{
    return v.toString();
}

template<>
bool from_string<dots::uuid>(const std::string& str, dots::uuid& v)
{
    return v.fromString(str);
}