#pragma once
#include <dots/type/StructDescriptor.h>

namespace dots
{

class Publisher
{
public:
    virtual void publish(const type::StructDescriptor<>* td, const type::Struct& instance, types::property_set_t what, bool remove) = 0;
};

}