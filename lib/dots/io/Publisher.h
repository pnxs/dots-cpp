#pragma once

namespace dots
{

namespace type {
class StructDescriptor;
}

class Publisher
{
public:
    virtual void publish(const type::StructDescriptor* td, CTypeless data, property_set what, bool remove) = 0;
};

}