#pragma once
#include <dots/serialization/formats/TextWriter.h>
#include <dots/serialization/formats/JsonFormat.h>

namespace dots::serialization
{
    using JsonWriter = TextWriter<JsonFormat>;
}
