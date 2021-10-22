#pragma once
#include <string>
#include <string_view>
#include <chrono>

namespace dots::type
{
    using DurationBase = std::chrono::duration<double>;

    struct Duration : DurationBase
    {
        using base_t = DurationBase;

        using base_t::base_t;
        Duration() = default;
        constexpr Duration(const base_t& duration) : base_t(duration){}
        constexpr explicit Duration(rep count) : base_t(count){}

        constexpr Duration(const Duration& other) = default;
        constexpr Duration(Duration&& other) noexcept = default;
        ~Duration() = default;

        constexpr Duration& operator = (const Duration& rhs) = default;
        constexpr Duration& operator = (Duration&& rhs) = default;

        using base_t::operator++;
        using base_t::operator--;

        using base_t::operator+=;
        using base_t::operator-=;
        using base_t::operator*=;
        using base_t::operator/=;
        using base_t::operator%=;

        using base_t::operator+;
        using base_t::operator-;

        constexpr explicit operator rep() const { return count(); }

        constexpr bool isZero() const { return *this == base_t::zero(); }
        constexpr double toFractionalSeconds() const { return std::chrono::duration_cast<std::chrono::duration<double>>(*this).count(); }
        constexpr int toSeconds() const { return static_cast<int>(std::chrono::round<std::chrono::seconds>(*this).count()); }
        constexpr int toMilliseconds() const { return static_cast<int>(std::chrono::round<std::chrono::milliseconds>(*this).count()); }
        constexpr int toMicroseconds() const { return static_cast<int>(std::chrono::round<std::chrono::microseconds>(*this).count()); }
        constexpr int toNanoseconds() const { return static_cast<int>(std::chrono::round<std::chrono::nanoseconds>(*this).count()); }

        std::string toString() const;
        bool fromString(const std::string_view& value);

        static Duration FromString(const std::string_view& value);
    };

    template <typename Base>
    struct TimePointImpl : Base
    {
        using base_t = Base;
        using duration_t = Duration;
        using clock_t = typename base_t::clock;
        static constexpr std::string_view ISO8601Duration = "I8601DUR";
        static constexpr std::string_view ISO8601DateTime = "I8601DT";
        static constexpr std::string_view DefaultFormat = clock_t::is_steady ? ISO8601Duration : ISO8601DateTime;

        using base_t::base_t;
        constexpr TimePointImpl() = default;
        constexpr TimePointImpl(const base_t& baseTimePoint) : base_t(baseTimePoint){}

        constexpr TimePointImpl(const TimePointImpl& other) = default;
        constexpr TimePointImpl(TimePointImpl&& other) noexcept = default;
        ~TimePointImpl() = default;

        constexpr TimePointImpl& operator = (const TimePointImpl& rhs) = default;
        constexpr TimePointImpl& operator = (TimePointImpl&& rhs) = default;

        using base_t::operator+=;
        using base_t::operator-=;

        constexpr duration_t duration() const { return base_t::time_since_epoch(); }
        constexpr bool isZero() const { return duration().isZero(); }

        std::string toString(const std::string_view& fmt = DefaultFormat, bool utc = false) const;
        bool fromString(const std::string_view& value, const std::string_view& fmt = DefaultFormat);

        static TimePointImpl FromString(const std::string_view& value, const std::string_view& fmt = DefaultFormat);

        static TimePointImpl Now()
        {
            return TimePointImpl{ clock_t::now() };
        }
    };

    using SystemTimePointBase = std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<double>>;
    using SteadyTimePointBase = std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double>>;

    extern template struct TimePointImpl<SystemTimePointBase>;
    extern template struct TimePointImpl<SteadyTimePointBase>;

    using TimePoint = TimePointImpl<SystemTimePointBase>;
    using SteadyTimePoint = TimePointImpl<SteadyTimePointBase>;

    namespace literals
    {
        using namespace std::chrono_literals;
    }
}