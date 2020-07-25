#pragma once

#undef LOG_EMERG_P
#undef LOG_INFO_P
#undef LOG_NOTICE_P
#undef LOG_WARN_P
#undef LOG_ERROR_P
#undef LOG_CRIT_P
#undef LOG_DEBUG_P
#undef LOG_DATA_P

#undef LOG_EMERG_S
#undef LOG_INFO_S
#undef LOG_NOTICE_S
#undef LOG_WARN_S
#undef LOG_ERROR_S
#undef LOG_CRIT_S
#undef LOG_DEBUG_S
#undef LOG_DATA_S

#ifdef ENABLE_LOG_DATA
#define LOG_DATA_S(...)  LOG_S(data,  FLF, __VA_ARGS__)
#define LOG_DATA_P(...)  LOG_P(data,  FLF, __VA_ARGS__)
#else
#define LOG_DATA_S(...)
#define LOG_DATA_P(FSTR, ...)
#endif

#ifdef ENABLE_LOG_DEBUG
#define LOG_DEBUG_S(...) LOG_S(debug, FLF, __VA_ARGS__)
#define LOG_DEBUG_P(...) LOG_P(debug, FLF, __VA_ARGS__)
#else
#define LOG_DEBUG_S(...)
#define LOG_DEBUG_P(FSTR, ...)
#endif

#define LOG_INFO_S(...)   LOG_S(info,  FLF, CEXPANSION << __VA_ARGS__)
#define LOG_NOTICE_S(...) LOG_S(notice,FLF, CEXPANSION << __VA_ARGS__)
#define LOG_WARN_S(...)   LOG_S(warn,  FLF, CEXPANSION << __VA_ARGS__)
#define LOG_ERROR_S(...)  LOG_S(error, FLF, CEXPANSION << __VA_ARGS__)
#define LOG_CRIT_S(...)   LOG_S(crit,  FLF, CEXPANSION << __VA_ARGS__)
#define LOG_EMERG_S(...)  LOG_S(emerg, FLF, CEXPANSION << __VA_ARGS__)

#define LOG_INFO_P(FSTR, ...)   LOG_P(info,  FLF, PEXPSTR_PRE FSTR PEXPARGS_PRE __VA_ARGS__)
#define LOG_NOTICE_P(FSTR, ...) LOG_P(notice,FLF, PEXPSTR_PRE FSTR PEXPARGS_PRE __VA_ARGS__)
#define LOG_WARN_P(FSTR, ...)   LOG_P(warn,  FLF, PEXPSTR_PRE FSTR PEXPARGS_PRE __VA_ARGS__)
#define LOG_ERROR_P(FSTR, ...)  LOG_P(error, FLF, PEXPSTR_PRE FSTR PEXPARGS_PRE __VA_ARGS__)
#define LOG_CRIT_P(FSTR, ...)   LOG_P(crit,  FLF, PEXPSTR_PRE FSTR PEXPARGS_PRE __VA_ARGS__)
#define LOG_EMERG_P(FSTR, ...)  LOG_P(emerg, FLF, PEXPSTR_PRE FSTR PEXPARGS_PRE __VA_ARGS__)
