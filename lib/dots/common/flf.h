#pragma once
#include <string_view>

namespace dots_util
{

/**
 * File line function.
 * Stores the information of a specific sourcecode point.
 */
class Flf
{
public:
    constexpr Flf() = default;
    constexpr Flf(const std::string_view& filePath, int l, const std::string_view fun): file(filePath.substr(filePath.find_last_of('/') + 1)), line(l), func(fun) {}

    constexpr bool valid() const { return file != nullptr; }

    std::string_view file;
    const int line = 0;
    std::string_view func;
};

}

#define FLF dots_util::Flf(__FILE__, __LINE__, __FUNCTION__)
