#pragma once
#include <dots/common/CTime.h>
#include <dots/common/Chrono.h>

namespace dots::type::posix
{
    struct TimeZone
    {
        TimeZone(const std::string& tz)
        {
            const char* tz_env = ::getenv("TZ");

            if (tz_env != NULL)
            {
                m_old_tz = tz_env;
                m_old_tz_valid = true;
            }

            setenv("TZ", tz.c_str(), 1);
            tzset();
        }

        ~TimeZone()
        {
            if (m_old_tz_valid)
            {
                setenv("TZ", m_old_tz.c_str(), 1);
            }
            else
            {
                unsetenv("TZ");
            }
            tzset();
        }

    private:

        std::string m_old_tz;
        bool m_old_tz_valid = false;
    };

    struct TimeZoneUTC : TimeZone
    {
        TimeZoneUTC() : TimeZone("UTC")
        {
        }
    };

    struct PosixTm : libc::Tm
    {
        PosixTm(time_t t, bool gm = false)
        {
            (gm ? gmtime_r : localtime_r)(&t, *this);
        }

        time_t mktime(bool gm)
        {
            if (gm)
            {
                TimeZoneUTC utc;
                std::tm* tm = *this;
                tm->tm_isdst = 0;

                return ::mktime(tm);
            }
            else
            {
                return Tm::mktime();
            }
        }

        size_t strftime(char* s, size_t max, const char* format) const
        {
            return ::strftime(s, max, format, *this);
        }

        char* strptime(const std::string& s, const std::string& format)
        {
            return ::strptime(s.c_str(), format.c_str(), *this);
        }
    };

    struct Timeval
    {
        static constexpr double MicrosecondsPerSecond = 1E6;

        constexpr Timeval() :
            m_timeval{}
        {}
        constexpr Timeval(const Timeval& other) = default;
        constexpr Timeval(Timeval&& other) noexcept = default;
        ~Timeval() = default;

        constexpr Timeval& operator = (const Timeval& rhs) = default;
        constexpr Timeval& operator = (Timeval&& rhs) noexcept = default;

        constexpr Timeval(const timeval& val) :
            m_timeval{ val }
        {
        }

        Timeval(const Duration& duration) :
            Timeval(timeval{ 
                static_cast<time_t>(::nearbyint(duration.toSeconds())),
                static_cast<suseconds_t>(::nearbyint((duration.toSeconds() - ::nearbyint(duration.toSeconds())) * MicrosecondsPerSecond))
            })
        {
        }

        operator const timeval* () const
        {
            return &m_timeval;
        }

        operator timeval* ()
        {
            return &m_timeval;
        }

        operator Duration () const
        {
            return Duration{ m_timeval.tv_sec + m_timeval.tv_usec / MicrosecondsPerSecond };
        }

    private:

        timeval m_timeval;
    };

    inline Timeval operator + (const Timeval& lhs, const Timeval& rhs)
    {
        return Duration{ lhs } + Duration{ rhs };
    }

    inline Timeval operator - (const Timeval& lhs, const Timeval& rhs)
    {
        return Duration{ lhs } - Duration{ rhs };
    }

    inline bool operator < (const Timeval& lhs, const Timeval& rhs)
    {
        return Duration{ lhs } < Duration{ rhs };
    }

    inline bool operator > (const Timeval& lhs, const Timeval& rhs)
    {
        return Duration{ lhs } > Duration{ rhs };
    }
}