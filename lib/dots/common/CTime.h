#pragma once
#include <ctime>

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

    class Timespec : public timespec
    {
    public:

        /// construct zero
        Timespec() :
            timespec{ 0, 0 }
        {
        }

        Timespec(const timespec& val) :
            timespec(val)
        {
        }

        /// construct from seconds since 1.1.1970
        Timespec(double seconds)
        {
            tv_sec = seconds;
            tv_nsec = nearbyint((seconds - tv_sec) * (1000 * 1000 * 1000));
        }

        double toSeconds() const
        {
            return tv_sec + tv_nsec / static_cast<double>(1000 * 1000 * 1000);
        }
    };
}