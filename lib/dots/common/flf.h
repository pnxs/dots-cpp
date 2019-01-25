#pragma once

namespace dots_util
{

constexpr const char * const strend(const char * const str) {
    return *str ? strend(str + 1) : str;
}

constexpr const char * const fromlastslash(const char * const start, const char * const end) {
    return (end >= start && *end != '/' && *end != '\\') ? fromlastslash(start, end - 1) : (end + 1);
}

constexpr const char * const pathlast(const char * const path) {
    return fromlastslash(path, strend(path));
}

/**
 * File line function.
 * Stores the information of a specific sourcecode point.
 */
class Flf
{
public:
    constexpr Flf() = default;
    constexpr Flf(const char *f, int l, const char *fun): file(f), line(l), func(fun) {}

    constexpr bool valid() const { return file != nullptr; }

    const char *file = nullptr;
    const int line = 0;
    const char *func = nullptr;
};

}

#define FLF dots_util::Flf(dots_util::pathlast(__FILE__), __LINE__, __FUNCTION__)
