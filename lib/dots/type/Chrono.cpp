#include <sstream>
#include <regex>
#include <charconv>
#include <dots/type/Chrono.h>
#include <date/date.h>
#include <date/tz.h>

namespace dots::type
{
    std::string Duration::toString() const
    {
        if (*this == zero())
        {
            return "PT0S";
        }

        std::ostringstream oss;

        oss << "P";

        Duration duration{ *this };

        auto floor_to_string = [&oss, &duration](auto floorZero, char c)
        {
            using duration_t = decltype(floorZero);

            if (auto floored = date::floor<duration_t>(duration); floored > floorZero)
            {
                oss << floored.count() << c;
                duration -= floored;
            }
        };

        floor_to_string(date::days::zero(), 'D');

        if (duration > zero())
        {
            oss << "T";
            floor_to_string(std::chrono::hours::zero(), 'H');
            floor_to_string(std::chrono::minutes::zero(), 'M');

            if (duration > zero())
            {
                oss << duration.count() << "S";
            }
        }

        return oss.str();
    }

    bool Duration::fromString(const std::string_view& value)
    {
        *this = FromString(value);
        return true;
    }

    Duration Duration::FromString(const std::string_view& value)
    {
        if (value == "PT0S")
        {
            return Duration{ zero() };
        }

        // note that this implementation is currently not able to parse durations without a time designator. additionally
        // it is pretty slow and should be overhauled at a later point
        std::regex regex{ R"(P([0-9\.]+Y)?([0-9\.]+M)?([0-9\.]+W)?([0-9\.]+D)?(T)([0-9\.]+H)?([0-9\.]+M)?([0-9\.]+S)?)", std::regex_constants::ECMAScript };
        std::smatch match;
        std::string input = value.data();

        if (!std::regex_match(input, match, regex) || match.size() <= 1)
        {
            throw std::runtime_error{ "could not parse duration from string: " + std::string{ value } };
        }

        bool preTimeDesignator = true;
        Duration duration = zero();

        for (size_t i = 1; i < match.size(); ++i)
        {
            std::ssub_match subMatch = match[i];
            std::string part = subMatch.str();

            if (part.empty())
            {
                continue;
            }
            
            auto parse_duration = [](auto duration, const std::string& part)
            {
                using duration_t = decltype(duration);
                using rep_t = typename duration_t::rep;

                rep_t count;

                if (std::from_chars_result result = std::from_chars(part.data(), part.data() + part.size(), count); result.ec != std::errc{})
				{
				    throw std::runtime_error{ "could not parse duration from string part: " + part  + " -> " + std::make_error_code(result.ec).message() };
				}

                return duration_t{ count };
            };

            switch (part.back())
            {
                case 'Y': duration += parse_duration(date::years::zero(), part); break;
                case 'W': duration += parse_duration(date::weeks::zero(), part); break;
                case 'D': duration += parse_duration(date::days::zero(), part); break;
                case 'T': preTimeDesignator = false; break;
                case 'H': duration += parse_duration(std::chrono::hours::zero(), part); break;
                case 'S': duration += std::chrono::duration_cast<std::chrono::nanoseconds>(base_t{ std::stod(part) }); break;
                case 'M':
                {
                    if (preTimeDesignator)
                    {
                        duration += parse_duration(date::months::zero(), part);
                    }
                    else
                    {
                        duration += parse_duration(std::chrono::minutes::zero(), part);
                    }

                    break;
                }
                default: throw std::runtime_error{ "encountered invalid symbol while parsing duration: '" + std::string{ part.back() } + "'" };
            }
        }

        return Duration{ duration };
    }

    using sys_time_t = date::sys_time<std::chrono::milliseconds>;
    using sys_duration_t = sys_time_t::duration;

    template <typename Base>
    std::string TimePointImpl<Base>::toString(const std::string_view& fmt/* = DefaultFormat*/, bool utc/* = false*/) const
    {
        if (fmt.empty())
        {
            return std::to_string(duration().count());
        }
        else
        {
            if (fmt == ISO8601Duration)
            {
                return duration().toString();
            }
            else
            {
                std::ostringstream oss;
                oss.exceptions(std::istringstream::failbit | std::istringstream::badbit);
                sys_time_t sysTimePoint{ std::chrono::duration_cast<sys_duration_t>(duration()) };

                auto time_point_to_stream = [](auto& oss, std::string_view fmt, auto timePoint)
                {
                    date::to_stream(oss, fmt == ISO8601DateTime ? "%FT%T%Ez" : fmt.data(), timePoint);
                };

                if (utc)
                {
                    time_point_to_stream(oss, fmt, sysTimePoint);
                }
                else
                {
                    date::zoned_time localTimePoint{ date::current_zone(), sysTimePoint };
                    time_point_to_stream(oss, fmt, localTimePoint.get_local_time());
                }

                return oss.str();
            }
        }
    }

    template <typename Base>
    bool TimePointImpl<Base>::fromString(const std::string_view& value, const std::string_view& fmt/* = DefaultFormat*/)
    {
        *this = FromString(value, fmt);
        return true;
    }

    template <typename Base>
    TimePointImpl<Base> TimePointImpl<Base>::FromString(const std::string_view& value, const std::string_view& fmt/* = DefaultFormat*/)
    {
        if (fmt.empty())
        {
            std::istringstream iss{ value.data() };
            iss.exceptions(std::istringstream::failbit | std::istringstream::badbit);
            duration_t::rep count;
            iss >> count;

            return TimePointImpl{ Duration{ count } };
        }
        else
        {
            if (fmt == ISO8601Duration)
            {
                return TimePointImpl{ Duration::FromString(value) };
            }
            else
            {
                std::istringstream iss{ value.data() };
                iss.exceptions(std::istringstream::failbit | std::istringstream::badbit);
                sys_time_t sysTimePoint;
                iss >> date::parse(fmt == ISO8601DateTime ? "%FT%T%Ez" : fmt.data(), sysTimePoint);

                return TimePointImpl{ sysTimePoint.time_since_epoch() };
            }
        }
    }

    template struct TimePointImpl<SystemTimePointBase>;
    template struct TimePointImpl<SteadyTimePointBase>;
}