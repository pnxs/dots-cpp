#include "dots/serialization/formats/CborReader.h"
#include <fmt/core.h>

namespace dots::serialization
{
    void CborReader::throwInvalidSize(const std::string& msg, std::size_t expected, std::size_t size) const
    {
        throwException(msg, fmt::format("Expected '{}' but got '{}'", expected, size));
    }

    void CborReader::throwUnexpectedHeadException(uint8_t expectedHead, uint8_t head) const
    {
        throwException("encountered unexpected head",
                       fmt::format("Expected {:02x} but got {:02x}", expectedHead, head));
    }

    void CborReader::throwUnexpectedMajorTypeException(uint8_t expectedMajorType, uint8_t majorType) const
    {
        throwException("encountered unexpected major type",
                       fmt::format("Expected '{:#02x}' but got '{:#02x}'",
                                   expectedMajorType,
                                   majorType));
    }

    void CborReader::throwException(const std::string& msg, int value) const
    {
        throwException(msg, fmt::format("value={}", value));
    }

    void CborReader::throwException(const std::string& msg, const std::string& details) const
    {
        std::size_t offset = std::distance(inputDataBegin(), inputData());
        throw SerializerException{msg,
                                  details,
                                  offset,
                                  std::vector(inputDataBegin(), inputDataEnd() + 1)};
    }

}