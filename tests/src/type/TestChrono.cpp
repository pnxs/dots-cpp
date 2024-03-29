// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/testing/gtest/gtest.h>
#include <date/date.h>
#include <date/tz.h>
#include <dots/type/Chrono.h>

using namespace dots::type::literals;

struct TestDuration : ::testing::Test
{
};

TEST_F(TestDuration, toString)
{
    {
        dots::type::Duration sut = dots::type::Duration::zero();
        sut += date::days{ 42 };
        sut += 3h;
        sut += 42s;

        EXPECT_EQ(sut.toString(), "P42DT3H42S");
    }

    {
        dots::type::Duration sut = dots::type::Duration::zero();
        sut += date::days{ 41 };
        sut += 23h;
        sut += 59min;
        sut += 59s;
        sut += 999ms;
        sut += 1000us;

        EXPECT_EQ(sut.toString(), "P42D");
    }

    {
        dots::type::Duration sut = dots::type::Duration::zero();
        sut += 87600h;
        sut += 100ms;

        EXPECT_EQ(sut.toString(), "P3650DT0.1S");
    }
}

TEST_F(TestDuration, fromString)
{
    {
        dots::type::Duration sut;
        sut.fromString("P616DT3H19M3.791078880S");

        EXPECT_EQ(date::floor<date::days>(sut), date::days{ 616 });
        sut -= date::floor<date::days>(sut);

        EXPECT_EQ(date::floor<std::chrono::hours>(sut), 3h);
        sut -= date::floor<std::chrono::hours>(sut);

        EXPECT_EQ(date::floor<std::chrono::minutes>(sut), 19min);
        sut -= date::floor<std::chrono::minutes>(sut);

        EXPECT_EQ(date::floor<std::chrono::seconds>(sut), 3s);
        sut -= date::floor<std::chrono::seconds>(sut);

        EXPECT_EQ(date::floor<std::chrono::milliseconds>(sut), 791ms);
        sut -= date::floor<std::chrono::milliseconds>(sut);

        EXPECT_EQ(date::floor<std::chrono::microseconds>(sut), 78us);
        sut -= date::floor<std::chrono::microseconds>(sut);

        EXPECT_EQ(date::floor<std::chrono::nanoseconds>(sut), 880ns);
    }

    {
        dots::type::Duration sut;
        sut.fromString("PT72H59M");

        EXPECT_EQ(date::floor<std::chrono::hours>(sut), 72h);
        sut -= date::floor<std::chrono::hours>(sut);

        EXPECT_EQ(date::floor<std::chrono::minutes>(sut), 59min);
        sut -= date::floor<std::chrono::minutes>(sut);
    }

    {
        dots::type::Duration sut;
        sut.fromString("P102M3W4DT");

        EXPECT_EQ(date::floor<date::months>(sut), date::months{ 102 });
        sut -= date::floor<date::months>(sut);

        EXPECT_EQ(date::floor<date::weeks>(sut), date::weeks{ 3 });
        sut -= date::floor<date::weeks>(sut);

        EXPECT_EQ(date::floor<date::days>(sut), date::days{ 4 });
        sut -= date::floor<date::days>(sut);
    }

    {
        dots::type::Duration sut;
        sut.fromString("P100YT0.00001S");

        EXPECT_EQ(date::floor<date::years>(sut), date::years{ 100 });
        sut -= date::floor<date::years>(sut);

        EXPECT_EQ(date::floor<std::chrono::microseconds>(sut), 10us);
    }

    EXPECT_THROW(dots::type::Duration::FromString("100YT0.00001S"), std::runtime_error);
    EXPECT_THROW(dots::type::Duration::FromString("P100Y0.00001S"), std::runtime_error);
    EXPECT_THROW(dots::type::Duration::FromString("P100YT0.00001"), std::runtime_error);
    EXPECT_THROW(dots::type::Duration::FromString("P100XT0.00001S"), std::runtime_error);
    EXPECT_THROW(dots::type::Duration::FromString("P100Y100YT0.00001S"), std::runtime_error);
    EXPECT_THROW(dots::type::Duration::FromString("P100YT100Y0.00001S"), std::runtime_error);
}

struct TestTimePoint : ::testing::Test
{
protected:

    static constexpr double UnixTimestampValue = 1583960877.500000;
    static constexpr char UnixTimestampString[] = "1583960877.500000";
    static constexpr char UnixTimestampISO8601UTC[] = "2020-03-11T21:07:57.500+00:00";
    static constexpr char UnixTimestampISO8601Z[] = "2020-03-11T21:07:57.500Z";
    static constexpr char UnixTimestampISO8601CET[] = "2020-03-11T22:07:57.500+01:00";
    static constexpr char UnixTimestampISO8601SRET[] = "2020-03-12T08:07:57.500+11:00";
};

TEST_F(TestTimePoint, toString)
{
    EXPECT_EQ(dots::type::TimePoint{ dots::type::Duration{ UnixTimestampValue } }.toString({}), UnixTimestampString);
    EXPECT_EQ(dots::type::TimePoint{ dots::type::Duration{ UnixTimestampValue } }.toString(dots::type::TimePoint::ISO8601DateTime, true), UnixTimestampISO8601UTC);
}

TEST_F(TestTimePoint, fromString)
{
    EXPECT_EQ(dots::type::TimePoint::FromString(UnixTimestampString, {}).duration().toFractionalSeconds(), UnixTimestampValue);
    EXPECT_EQ(dots::type::TimePoint::FromString(UnixTimestampISO8601UTC).duration().toFractionalSeconds(), UnixTimestampValue);
    EXPECT_EQ(dots::type::TimePoint::FromString(UnixTimestampISO8601Z).duration().toFractionalSeconds(), UnixTimestampValue);
    EXPECT_EQ(dots::type::TimePoint::FromString(UnixTimestampISO8601CET).duration().toFractionalSeconds(), UnixTimestampValue);
    EXPECT_EQ(dots::type::TimePoint::FromString(UnixTimestampISO8601SRET).duration().toFractionalSeconds(), UnixTimestampValue);
}

struct TestSteadyTimePoint : ::testing::Test
{
protected:

    static constexpr double SteadyTimestampValue = 324702.125000;
    static constexpr char SteadyTimestampString[] = "324702.125000";
    static constexpr char  SteadyTimestampISO8601[] = "P3DT18H11M42.125S";
};

TEST_F(TestSteadyTimePoint, toString)
{
    EXPECT_EQ(dots::type::SteadyTimePoint{ dots::type::Duration{ SteadyTimestampValue } }.toString({}), SteadyTimestampString);
    EXPECT_EQ(dots::type::SteadyTimePoint{ dots::type::Duration{ SteadyTimestampValue } }.toString(), SteadyTimestampISO8601);
}

TEST_F(TestSteadyTimePoint, fromString)
{
    EXPECT_EQ(dots::type::SteadyTimePoint::FromString(SteadyTimestampString, {}).duration().toFractionalSeconds(), SteadyTimestampValue);
    EXPECT_EQ(dots::type::SteadyTimePoint::FromString(SteadyTimestampISO8601).duration().toFractionalSeconds(), SteadyTimestampValue);
}
