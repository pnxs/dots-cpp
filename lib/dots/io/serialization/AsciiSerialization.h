#pragma once

#include "dots/cpp_config.h"
#include <dots/type/StructDescriptor.h>

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
    dots::property_set highlightAttributes;
};


std::string to_ascii(const dots::type::StructDescriptor* td, const void* data, property_set properties = PROPERTY_SET_ALL, const ToAsciiOptions& cs = {});

}

