#pragma once

#include "StandardTypes.h"
#include "dots/Uuid.h"

namespace dots
{

namespace type
{

class UuidDescriptor: public StandardTypeDescriptor<dots::uuid>
{
public:
    UuidDescriptor(const std::string& dotsName)
        :StandardTypeDescriptor<dots::uuid>(dotsName, DotsType::uuid) {}
};

}

}

template<>
std::string to_string<dots::uuid>(const dots::uuid& v);

template<>
bool from_string<dots::uuid>(const std::string& str, dots::uuid& v);