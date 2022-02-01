#pragma once
#include <dots/serialization/formats/TextWriter.h>
#include <dots/serialization/formats/StringFormat.h>

namespace dots::serialization
{
    using StringWriter = TextWriter<StringFormat>;
}
