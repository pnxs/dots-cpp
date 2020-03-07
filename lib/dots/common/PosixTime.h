#pragma once
#include <string_view>
#include <dots/common/CTime.h>
#include <dots/common/Chrono.h>

namespace dots::type::posix
{
    struct ScopedTimeZone
    {
        ScopedTimeZone(const std::string_view& timeZone)
        {
            std::string_view currentTimeZone = ::getenv("TZ");

            if (!currentTimeZone.empty())
            {
                m_previousTimeZone = currentTimeZone;
            }

            ::setenv("TZ", timeZone.data(), true);
            ::tzset();
        }

        ~ScopedTimeZone()
        {
            if (m_previousTimeZone.empty())
            {
                ::unsetenv("TZ");
            }
            else
            {
                ::setenv("TZ", m_previousTimeZone.data(), true);
            }

            ::tzset();
        }

    private:

        std::string m_previousTimeZone;
    };

    struct PosixTm : libc::Tm
    {
        PosixTm(time_t time, bool utc = false)
        {
            (utc ? ::gmtime_r : ::localtime_r)(&time, *this);
        }
        constexpr PosixTm(const PosixTm& other) = default;
        constexpr PosixTm(PosixTm&& other) noexcept = default;
        ~PosixTm() = default;

        constexpr PosixTm& operator = (const PosixTm& rhs) = default;
        constexpr PosixTm& operator = (PosixTm&& rhs) noexcept = default;

        time_t mktime(bool utc)
        {
            if (utc)
            {
                ScopedTimeZone scopedTimeZone{ "UTC" };
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
        {
        }
        constexpr Timeval(const Timeval& other) = default;
        constexpr Timeval(Timeval&& other) noexcept = default;
        ~Timeval() = default;

        constexpr Timeval& operator = (const Timeval& rhs) = default;
        constexpr Timeval& operator = (Timeval&& rhs) noexcept = default;

        constexpr Timeval(const timeval& val) :
            m_timeval{ val }
        {
        }

        constexpr Timeval(const Duration& duration) :
            Timeval(timeval{ 
                static_cast<time_t>(duration.toSeconds()),
                static_cast<suseconds_t>(::nearbyint((duration.count() - duration.toSeconds()) * MicrosecondsPerSecond))
            })
        {
        }

        constexpr operator const timeval* () const
        {
            return &m_timeval;
        }

        constexpr operator timeval* ()
        {
            return &m_timeval;
        }

        constexpr operator Duration () const
        {
            return Duration{ m_timeval.tv_sec + m_timeval.tv_usec / MicrosecondsPerSecond };
        }

    private:

        timeval m_timeval;
    };

    constexpr Timeval operator + (const Timeval& lhs, const Timeval& rhs)
    {
        return Timeval{ Duration{ lhs } + Duration{ rhs } };
    }

    constexpr Timeval operator - (const Timeval& lhs, const Timeval& rhs)
    {
        return Timeval{ Duration{ lhs } - Duration{ rhs } };
    }

    constexpr bool operator < (const Timeval& lhs, const Timeval& rhs)
    {
        return Duration{ lhs } < Duration{ rhs };
    }

    constexpr bool operator > (const Timeval& lhs, const Timeval& rhs)
    {
        return Duration{ lhs } > Duration{ rhs };
    }
}