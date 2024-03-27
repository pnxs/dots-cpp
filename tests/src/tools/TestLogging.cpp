// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/testing/gtest/gtest.h>
#include <vector>
#include <dots/tools/logging.h>
#include <dots/fmt/logging_fmt.h>

using ::testing::MatchesRegex;

struct TestLogging : ::testing::Test
{
};

TEST_F(TestLogging, test_levels)
{
    int iterations = 1;
    ::testing::internal::CaptureStderr();
    LOG_INFO_S("Info" << "Warmup")

    std::string str1 = "name";
    std::string str2 = "log";

    dots::tools::loggingFrontend().setLogLevel(1);

    auto t1 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i)
    {
        LOG_DATA_S(str1 << " Data " << str2)
        LOG_DEBUG_S(str1 << " Debug " << str2)
        LOG_INFO_S(str1 << " Info " << str2)
        LOG_NOTICE_S(str1 << " Notice " << str2)
        LOG_WARN_S(str1 << " Warn " << str2)
        LOG_ERROR_S(str1 << " Error " << str2)
        LOG_CRIT_S(str1 << " Critical " << str2)
        LOG_EMERG_S(str1 << " Emergency " << str2)
    }

    auto t2 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i)
    {
        LOG_DATA_P("%s Data %s",str1.c_str(), str2.c_str())
        LOG_DEBUG_P("%s Debug %s",str1.c_str(), str2.c_str())
        LOG_INFO_P("%s Info %s",str1.c_str(), str2.c_str())
        LOG_NOTICE_P("%s Notice %s",str1.c_str(), str2.c_str())
        LOG_WARN_P("%s Warn %s",str1.c_str(), str2.c_str())
        LOG_ERROR_P("%s Error %s",str1.c_str(), str2.c_str())
        LOG_CRIT_P("%s Critical %s",str1.c_str(), str2.c_str())
        LOG_EMERG_P("%s Emergency %s",str1.c_str(), str2.c_str())
    }
    auto t3 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i)
    {
        LOG_DATA_F("{} Data {}", str1, str2);
        LOG_DEBUG_F("{} Debug {}", str1, str2);
        LOG_INFO_F("{} Info {}", str1, str2);
        LOG_NOTICE_F("{} Notice {}", str1, str2);
        LOG_WARN_F("{} Warn {}", str1, str2);
        LOG_ERROR_F("{} Error {}", str1, str2);
        LOG_CRIT_F("{} Critical {}", str1, str2);
        LOG_EMERG_F("{} Emergency {}", str1, str2);
    }
    auto t4 = std::chrono::high_resolution_clock::now();

    std::string logout = testing::internal::GetCapturedStderr();

    auto tStream = t2 - t1;
    auto tPrintf = t3 - t2;
    auto tFmt = t4 - t3;

    fmt::print("tStream: {}\n", tStream.count());
    fmt::print("tPrintf: {}\n", tPrintf.count());
    fmt::print("tFmt...: {}\n", tFmt.count());
}

TEST_F(TestLogging, test_consolebackend_bw_flf)
{
    ::testing::internal::CaptureStderr();
    setenv("DOTS_DISABLE_LOG_COLORS", "1", 1);
    setenv("DOTS_LOG_FLF", "1", 1);

    dots::tools::ConsoleLogBackend consoleLogBackend;

    auto flf = dots::tools::Flf("file", 42, "function");
    consoleLogBackend.log(dots::tools::Level::info, flf, "message" );

    std::string logout = testing::internal::GetCapturedStderr();

    EXPECT_THAT(logout, MatchesRegex("info  : \\[[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}.[0-9]{3}\\+[0-9]{2}:[0-9]{2}\\] message \\(file:42 \\(function\\)\\)\n"));
}

TEST_F(TestLogging, test_consolebackend_bw)
{
    ::testing::internal::CaptureStderr();
    setenv("DOTS_DISABLE_LOG_COLORS", "1", 1);
    unsetenv("DOTS_LOG_FLF");

    dots::tools::ConsoleLogBackend consoleLogBackend;

    auto flf = dots::tools::Flf("file", 42, "function");
    consoleLogBackend.log(dots::tools::Level::info, flf, "message" );

    std::string logout = testing::internal::GetCapturedStderr();

    EXPECT_THAT(logout, MatchesRegex("info  : \\[[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}.[0-9]{3}\\+[0-9]{2}:[0-9]{2}\\] message\n"));
}
