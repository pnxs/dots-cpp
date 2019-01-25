#pragma once

#include "StandardTypes.h"
#include <dots/eventloop/Chrono.h>

namespace dots
{

using pnxs::TimePoint;
using pnxs::SteadyTimePoint;
using pnxs::Duration;

namespace type
{

template<class T>
class ChronoDescriptor: public StandardTypeDescriptor<T>
{
public:
    ChronoDescriptor(const std::string& dotsName)
        :StandardTypeDescriptor<T>(dotsName) {}
};

template<>
class ChronoDescriptor<Duration>: public StandardTypeDescriptor<Duration>
{
public:
    ChronoDescriptor(const std::string& dotsName)
        :StandardTypeDescriptor<Duration>(dotsName, DotsType::duration) {}
};

template<>
class ChronoDescriptor<TimePoint>: public StandardTypeDescriptor<TimePoint>
{
public:
    ChronoDescriptor(const std::string& dotsName)
        :StandardTypeDescriptor<TimePoint>(dotsName, DotsType::timepoint) {};
};

template<>
class ChronoDescriptor<SteadyTimePoint>: public StandardTypeDescriptor<SteadyTimePoint>
{
public:
    ChronoDescriptor(const std::string& dotsName)
        :StandardTypeDescriptor<SteadyTimePoint>(dotsName, DotsType::steady_timepoint) {}
};

typedef ChronoDescriptor<Duration> DurationDescriptor;
typedef ChronoDescriptor<TimePoint> TimePointDescriptor;
typedef ChronoDescriptor<SteadyTimePoint> SteadyTimePointDescriptor;

}

//extern template
//class StandardTypeDescriptor<Duration>;

}

template<>
std::string to_string<pnxs::Duration>(const pnxs::Duration& v);

template<>
bool from_string<pnxs::Duration>(const std::string& str, pnxs::Duration& v);

template<>
std::string to_string<pnxs::TimePoint>(const pnxs::TimePoint& v);

template<>
bool from_string<pnxs::TimePoint>(const std::string& str, pnxs::TimePoint& v);

template<>
std::string to_string<pnxs::SteadyTimePoint>(const pnxs::SteadyTimePoint& v);

template<>
bool from_string<pnxs::SteadyTimePoint>(const std::string& str, pnxs::SteadyTimePoint& v);