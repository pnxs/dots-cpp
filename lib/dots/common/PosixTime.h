#pragma once
#include <dots/common/CTime.h>

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

    class Timeval : public timeval
    {
    public:

        Timeval() :
            timeval{ 0, 0 }
        {
        }

        Timeval(const timeval& val) :
            timeval(val)
        {
        }

        Timeval(double seconds)
        {
            tv_sec = seconds;
            tv_usec = nearbyint((seconds - tv_sec) * 1000000);
        }

        /// convert Timeval to Seconds
        double toSeconds() const
        {
            return tv_sec + tv_usec / static_cast<double>(1000000);
        }
    };
}