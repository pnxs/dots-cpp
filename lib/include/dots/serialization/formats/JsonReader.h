#pragma once
#include <dots/serialization/formats/TextReader.h>
#include <dots/serialization/formats/JsonFormat.h>

namespace dots::serialization
{
    using JsonReader = TextReader<JsonFormat>;
}
