#pragma once
#include <ctime>
#include <dots/type/Chrono.h>

namespace dots::type::libc
{
    struct Tm
    {
        Tm() = default;

        constexpr Tm(const std::tm& val) :
            m_tm(val)
        {
        }

        constexpr Tm(const Tm& other) = default;
        constexpr Tm(Tm&& other) noexcept = default;
        ~Tm() = default;

        constexpr Tm& operator = (const Tm& rhs) = default;
        constexpr Tm& operator = (Tm&& rhs) noexcept = default;

        constexpr operator const std::tm* () const
        {
            return &m_tm;
        }

        constexpr operator std::tm* ()
        {
            return &m_tm;
        }

        time_t mktime()
        {
            m_tm.tm_isdst = -1;
            return ::mktime(&m_tm);
        }

        constexpr int year() const { return m_tm.tm_year + 1900; }
        constexpr int month() const { return m_tm.tm_mon + 1; }
        constexpr int day() const { return m_tm.tm_mday; }
        constexpr int weekDay() const { return m_tm.tm_wday + 1; }
        constexpr int hour() const { return m_tm.tm_hour; }
        constexpr int minute() const { return m_tm.tm_min; }
        constexpr int second() const { return m_tm.tm_sec; }

        constexpr void year(int val) { m_tm.tm_year = val - 1900; }
        constexpr void month(int val) { m_tm.tm_mon = val - 1; }
        constexpr void day(int val) { m_tm.tm_mday = val; }
        constexpr void weekDay(int val) { m_tm.tm_wday = val - 1; }
        constexpr void hour(int val) { m_tm.tm_hour = val; }
        constexpr void minute(int val) { m_tm.tm_min = val; }
        constexpr void second(int val) { m_tm.tm_sec = val; }

    private:

        std::tm m_tm;
    };

    struct Timespec
    {
        static constexpr double NanosecondsPerSecond = 1E9;

        constexpr Timespec() :
            m_timespec{}
        {
        }
        constexpr Timespec(const Timespec& other) = default;
        constexpr Timespec(Timespec&& other) noexcept = default;
        ~Timespec() = default;

        constexpr Timespec& operator = (const Timespec& rhs) = default;
        constexpr Timespec& operator = (Timespec&& rhs) noexcept = default;

        constexpr Timespec(const timespec& val) :
            m_timespec{ val }
        {
        }

        constexpr Timespec(Duration duration) :
            Timespec(timespec{
                static_cast<std::time_t>(duration.toSeconds()),
                static_cast<long>(::nearbyint((duration.toFractionalSeconds() - duration.toSeconds()) * NanosecondsPerSecond))
            })
        {
        }

        constexpr Timespec(TimePoint timePoint) :
            Timespec(timePoint.duration())
        {
        }

        constexpr Timespec(SteadyTimePoint steadyTimePoint) :
            Timespec(steadyTimePoint.duration())
        {
        }

        constexpr operator const timespec* () const
        {
            return &m_timespec;
        }

        constexpr operator timespec* ()
        {
            return &m_timespec;
        }

        constexpr operator Duration () const
        {
            return Duration{ m_timespec.tv_sec + m_timespec.tv_nsec / NanosecondsPerSecond };
        }

    private:

        timespec m_timespec;
    };

    constexpr Timespec operator + (const Timespec& lhs, const Timespec& rhs)
    {
        return Timespec{ Duration{ lhs } + Duration{ rhs } };
    }

    constexpr Timespec operator - (const Timespec& lhs, const Timespec& rhs)
    {
        return Timespec{ Duration{ lhs } - Duration{ rhs } };
    }

    constexpr bool operator < (const Timespec& lhs, const Timespec& rhs)
    {
        return Duration{ lhs } < Duration{ rhs };
    }

    constexpr bool operator > (const Timespec& lhs, const Timespec& rhs)
    {
        return Duration{ lhs } > Duration{ rhs };
    }
}