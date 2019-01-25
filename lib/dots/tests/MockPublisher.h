#pragma once

#include "gmock/gmock.h"
#include "dots/io/Publisher.h"
#include "dots/type/StructDescriptor.h"

namespace dots {

class MockPublisher : public dots::Publisher
{
public:
    MOCK_METHOD4(publish, void(
            const type::StructDescriptor *td, CTypeless data,
                    property_set what, bool remove));
};

}