#pragma once

#include "gmock/gmock.h"
#include "dots/io/Publisher.h"
#include "dots/type/NewStructDescriptor.h"

namespace dots {

class MockPublisher : public dots::Publisher
{
public:
    MOCK_METHOD4(publish, void(
            const type::StructDescriptor<> *td, const type::Struct& instance,
                    type::PropertySet what, bool remove));
};

}