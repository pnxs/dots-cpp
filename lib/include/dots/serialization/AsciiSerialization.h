#pragma once

#include <dots/type/StructDescriptor.h>
#include <dots/type/FundamentalTypes.h>

namespace dots {



struct ToAsciiColorSchema
{
    virtual ~ToAsciiColorSchema() = default;

    virtual const char* string() const = 0;
    virtual const char* integer() const = 0;
    virtual const char* floatingPoint() const = 0;
    virtual const char* enumValue() const = 0;
    virtual const char* attribute() const = 0;
    virtual const char* typeName() const = 0;
    virtual const char* allOff() const = 0;
    virtual const char* create() const = 0;
    virtual const char* update() const = 0;
    virtual const char* remove() const = 0;
    virtual const char* timestamp() const = 0;
    virtual const char* highlight() const = 0;
};

struct ToAsciiOptions
{
    ToAsciiColorSchema* cs = nullptr;
    bool singleLine = false;
    bool enumAsTag = false;
    property_set_t highlightAttributes;
};


std::string to_ascii(const type::StructDescriptor<>* td, const void* data, property_set_t properties = property_set_t::All, const ToAsciiOptions& cs = {});

}

