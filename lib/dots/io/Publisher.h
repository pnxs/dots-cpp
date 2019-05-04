#pragma once

namespace dots
{

namespace type {
class StructDescriptor;
}

class Publisher
{
public:
    virtual void publish(const type::StructDescriptor* td, const type::Struct& instance, property_set what, bool remove) = 0;
};

}