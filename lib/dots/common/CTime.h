#pragma once
#include <ctime>
#include <dots/common/Chrono.h>

namespace dots::type::libc
{
    struct Tm
    {
        Tm() = default;

        Tm(const std::tm& val) :
            m_tm(val)
        {
        }

        operator const std::tm* () const
        {
            return &m_tm;
        }

        operator std::tm* ()
        {
            return &m_tm;
        }

        time_t mktime()
        {
            m_tm.tm_isdst = -1;
            return ::mktime(&m_tm);
        }

        int year() const { return m_tm.tm_year + 1900; }
        int mon() const { return m_tm.tm_mon + 1; }
        int day() const { return m_tm.tm_mday; }
        int wday() const { return m_tm.tm_wday; }

        int hour() const { return m_tm.tm_hour; }
        int min() const { return m_tm.tm_min; }
        int sec() const { return m_tm.tm_sec; }

        void setYear(int val) { m_tm.tm_year = val - 1900; }
        void setMon(int val) { m_tm.tm_mon = val - 1; }
        void setDay(int val) { m_tm.tm_mday = val; }
        void setHour(int val) { m_tm.tm_hour = val; }
        void setMin(int val) { m_tm.tm_min = val; }
        void setSec(int val) { m_tm.tm_sec = val; }

    private:

        std::tm m_tm;
    };

    struct Timespec
    {
        static constexpr double NanosecondsPerSecond = 1E9;

        constexpr Timespec() :
            m_timespec{}
        {}
        constexpr Timespec(const Timespec& other) = default;
        constexpr Timespec(Timespec&& other) noexcept = default;
        ~Timespec() = default;

        constexpr Timespec& operator = (const Timespec& rhs) = default;
        constexpr Timespec& operator = (Timespec&& rhs) noexcept = default;

        constexpr Timespec(const timespec& val) :
            m_timespec{ val }
        {
        }

        Timespec(const Duration& duration) :
            Timespec(timespec{ 
                static_cast<std::time_t>(::nearbyint(duration.toSeconds())),
                static_cast<long>(::nearbyint((duration.toSeconds() - ::nearbyint(duration.toSeconds())) * NanosecondsPerSecond))
            })
        {
        }

        Timespec(const TimePoint& timePoint) :
            Timespec(timePoint.value())
        {
        }

        Timespec(const SteadyTimePoint& steadyTimePoint) :
            Timespec(steadyTimePoint.value())
        {
        }

        operator const timespec* () const
        {
            return &m_timespec;
        }

        operator timespec* ()
        {
            return &m_timespec;
        }

        operator Duration () const
        {
            return Duration{ m_timespec.tv_sec + m_timespec.tv_nsec / NanosecondsPerSecond };
        }

    private:

        timespec m_timespec;
    };

    inline Timespec operator + (const Timespec& lhs, const Timespec& rhs)
    {
        return Duration{ lhs } + Duration{ rhs };
    }

    inline Timespec operator - (const Timespec& lhs, const Timespec& rhs)
    {
        return Duration{ lhs } - Duration{ rhs };
    }

    inline bool operator < (const Timespec& lhs, const Timespec& rhs)
    {
        return Duration{ lhs } < Duration{ rhs };
    }

    inline bool operator > (const Timespec& lhs, const Timespec& rhs)
    {
        return Duration{ lhs } > Duration{ rhs };
    }
}