#pragma once

#include <dots/tools/logging.h>
#include <fmt/core.h>

#define LOG_F(level, flf, fmtstr, ...) \
{ \
using namespace dots::tools; \
if(loggingFrontend().shouldLog(Level::level)) \
{ \
    loggingFrontend().log(Level::level, flf, fmt::format(fmtstr, __VA_ARGS__)); \
} \
}

/**
 * If you want to compile in data-level logs, you have to define DOTS_ENABLE_LOG_DATA
 */
#ifdef DOTS_ENABLE_LOG_DATA
#define LOG_DATA_F(FMT, ...)  LOG_F(data,  FLF, FMT, __VA_ARGS__)
#else
#define LOG_DATA_F(...)
#endif

/**
 * If you want to compile in debug-level logs, you have to define DOTS_ENABLE_LOG_DATA
 */
#ifdef DOTS_ENABLE_LOG_DEBUG
#define LOG_DEBUG_F(FMT, ...) LOG_F(debug, FLF, FMT, __VA_ARGS__)
#else
#define LOG_DEBUG_F(...)
#endif

#define LOG_INFO_F(FMT, ...)   LOG_F(info,  FLF, FMT, __VA_ARGS__)
#define LOG_NOTICE_F(FMT, ...) LOG_F(notice,FLF, FMT, __VA_ARGS__)
#define LOG_WARN_F(FMT, ...)   LOG_F(warn,  FLF, FMT, __VA_ARGS__)
#define LOG_ERROR_F(FMT, ...)  LOG_F(error, FLF, FMT, __VA_ARGS__)
#define LOG_CRIT_F(FMT, ...)   LOG_F(crit,  FLF, FMT, __VA_ARGS__)
#define LOG_EMERG_F(FMT, ...)  LOG_F(emerg, FLF, FMT, __VA_ARGS__)
