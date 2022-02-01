#pragma once
#include <dots/serialization/formats/TextReader.h>
#include <dots/serialization/formats/StringFormat.h>

namespace dots::serialization
{
    using StringReader = TextReader<StringFormat>;
}
