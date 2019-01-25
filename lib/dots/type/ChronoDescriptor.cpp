#include "ChronoDescriptor.h"

template<>
std::string to_string<pnxs::Duration>(const pnxs::Duration& v)
{
    return v.toString();
}

template<>
bool from_string<pnxs::Duration>(const std::string& str, pnxs::Duration& v)
{
    return v.fromString(str);
}

template<>
std::string to_string<pnxs::TimePoint>(const pnxs::TimePoint& v)
{
    return v.toString();
}

template<>
bool from_string<pnxs::TimePoint>(const std::string& str, pnxs::TimePoint& v)
{
    return v.fromString(str);
}

template<>
std::string to_string<pnxs::SteadyTimePoint>(const pnxs::SteadyTimePoint& v)
{
    return v.toString();
}

template<>
bool from_string<pnxs::SteadyTimePoint>(const std::string& str, pnxs::SteadyTimePoint& v)
{
    return v.fromString(str);
}

namespace dots
{
namespace type
{

template
class StandardTypeDescriptor<Duration>;

template
class StandardTypeDescriptor<TimePoint>;

template
class StandardTypeDescriptor<SteadyTimePoint>;

}
}