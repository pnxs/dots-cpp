// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once

#include <cstdarg>
#include <sstream>
#include <mutex>
#include <memory>
#include <optional>

#include "flf.h"

/**
 * Simple logging system using macro-syntax
 *
 * LOG_<LEVEL>_S for logging using ostream and
 * LOG_<LEVEL>_P for printf-syntax
 *
 * Allowed levels are (like in syslog):
 *      DATA - lowlevel-debug message
 *      DEBUG - debug-level message
 *      INFO - informational message
 *      NOTICE - normal, but significant, condition
 *      WARN - warning conditions
 *      ERROR - error conditions
 *      CRIT - critical conditions
 *      EMERG - system is unusable
 *
 * Example
 *
 * LOG_INFO_S("Hello world, here is a " << steamable-object << "!")
 * LOG_INFO_P("Hello wordd, here is a #%d %s syntax!", 1, "printf")
 *
 * Select loglevel on application-start using environment variable DOTS_LOG_LEVEL:
 *
 * Example:
 * DOTS_LOG_LEVEL=1 (set loglevel to DATA)
 * DOTS_LOG_LEVEL=4 (set loglevel to NOTICE)
 *
 * Select log loggingBackend using environment variable DOTS_LOG_BACKEND:
 * Logging is written to console by default.
 *
 * Example:
 * DOTS_LOG_BACKEND=syslog (log to syslog)
 *
 * When logging to console, you can disable colors by setting
 * DOTS_DISABLE_LOG_COLORS=1
 * unset variable to enable colors again.
 */

namespace dots::tools
{
    enum class Level: uint8_t {
        data = 1,
        debug = 2,
        info = 3,
        notice = 4,
        warn = 5,
        error = 6,
        crit  = 7,
        emerg = 8
    };

    class LogBackend
    {
    public:
        virtual ~LogBackend() = default;

        virtual void log_p(Level level, const Flf &flf, const char* text) = 0;
        virtual void log(Level level, const Flf &flf, const std::string& text) = 0;
    };

    class LogFrontend
    {
    public:
        LogFrontend();
        void setLogLevel(Level level);
        void setLogLevel(int level);
        [[nodiscard]] Level getLogLevel() const;
        [[nodiscard]] bool shouldLog(Level level) const;
        static void log_p(Level level, const Flf &flf, const char* text);
        static void log_s(Level level, const Flf &flf, const std::ostringstream& text);

    private:
        static std::optional<Level> get_loglevel_from_env();
        static Level get_default_loglevel();

        Level m_level = Level::info;
    };

    class ConsoleLogBackend: public LogBackend
    {
    public:
        ConsoleLogBackend();

        void log_p(Level level, const Flf &flf, const char* text) override;
        void log(Level level, const Flf &flf, const std::string& text) override;
        static const char *level2color(Level level);

    private:
        bool m_colorOut = false;
        static constexpr int MaxLengthLevel = 6;
        std::mutex m_mutex;
    };

#ifdef __unix__
    class SyslogBackend: public LogBackend
    {
    public:
        explicit SyslogBackend(const char* ident = nullptr, int option = 0);
        ~SyslogBackend() override;

        void log_p(Level level, const Flf &flf, const char* messagea) override;
        void log(Level level, const Flf &flf, const std::string& text) override;

        static int toSyslogLevel(Level level);
    private:
        std::mutex m_mutex;
    };
#endif
    std::optional<Level> nr2level(uint8_t nr);
    const char *level2string(Level level);

    extern std::shared_ptr<LogBackend> g_loggingBackend;
    LogFrontend& loggingFrontend();
    LogBackend& loggingBackend();
}

#define LOG_P(level, flf, ...) \
{ \
using namespace dots::tools; \
if(loggingFrontend().shouldLog(Level::level)) \
{ \
    char buffer[8192]; \
    snprintf(buffer, sizeof(buffer)-1, __VA_ARGS__); \
    loggingFrontend().log_p(Level::level, flf, buffer); \
} \
}

#define LOG_S(level, flf, ...) \
{ \
using namespace dots::tools; \
if(loggingFrontend().shouldLog(Level::level)) \
{ \
    std::ostringstream os; \
    os << __VA_ARGS__; \
    loggingFrontend().log_s(Level::level, flf, os); \
} \
}

/**
 * If you want to compile in data-level logs, you have to define DOTS_ENABLE_LOG_DATA
 */
#ifdef DOTS_ENABLE_LOG_DATA
#define LOG_DATA_S(...)  LOG_S(data,  FLF, __VA_ARGS__)
#define LOG_DATA_P(...)  LOG_P(data,  FLF, __VA_ARGS__)
#else
#define LOG_DATA_S(...)
#define LOG_DATA_P(...)
#endif

/**
 * If you want to compile in debug-level logs, you have to define DOTS_ENABLE_LOG_DATA
 */
#ifdef DOTS_ENABLE_LOG_DEBUG
#define LOG_DEBUG_S(...) LOG_S(debug, FLF, __VA_ARGS__)
#define LOG_DEBUG_P(...) LOG_P(debug, FLF, __VA_ARGS__)
#else
#define LOG_DEBUG_S(...)
#define LOG_DEBUG_P(...)
#endif

#define LOG_INFO_S(...)  LOG_S(info,  FLF, __VA_ARGS__)
#define LOG_NOTICE_S(...)  LOG_S(notice,  FLF, __VA_ARGS__)
#define LOG_WARN_S(...)  LOG_S(warn,  FLF, __VA_ARGS__)
#define LOG_ERROR_S(...) LOG_S(error, FLF, __VA_ARGS__)
#define LOG_CRIT_S(...)  LOG_S(crit,  FLF, __VA_ARGS__)
#define LOG_EMERG_S(...) LOG_S(emerg, FLF, __VA_ARGS__)

#define LOG_INFO_P(...)  LOG_P(info,  FLF, __VA_ARGS__)
#define LOG_NOTICE_P(...)  LOG_P(notice,  FLF, __VA_ARGS__)
#define LOG_WARN_P(...)  LOG_P(warn,  FLF, __VA_ARGS__)
#define LOG_ERROR_P(...) LOG_P(error, FLF, __VA_ARGS__)
#define LOG_CRIT_P(...)  LOG_P(crit,  FLF, __VA_ARGS__)
#define LOG_EMERG_P(...) LOG_P(emerg, FLF, __VA_ARGS__)
