#pragma once
#include <dots/type/NewStructDescriptor.h>

namespace dots
{

class Publisher
{
public:
    virtual void publish(const type::NewStructDescriptor<>* td, const type::NewStruct& instance, types::property_set_t what, bool remove) = 0;
};

}