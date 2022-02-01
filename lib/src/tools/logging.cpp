#include <dots/tools/logging.h>
#ifdef __unix__
#include <syslog.h>
#endif
#include <dots/type/Chrono.h>

namespace dots::tools
{
    // LogFrontend
    LogFrontend::LogFrontend()
    {
        auto log_level = get_default_loglevel();

        auto env_log_level = get_loglevel_from_env();
        if (env_log_level)
        {
            log_level = env_log_level.value();
        }

        setLogLevel(log_level);
    }

    void LogFrontend::setLogLevel(Level level)
    {
        m_level = level;
    }

    Level LogFrontend::getLogLevel() const
    {
        return m_level;
    }

    bool LogFrontend::shouldLog(Level level) const
    {
        return level >= m_level;
    }

    void LogFrontend::log_p(Level level, const Flf &flf, const char* text)
    {
        loggingBackend().log_p(level, flf, text);
    }

    void LogFrontend::log_s(Level level, const Flf &flf, const std::ostringstream& text)
    {
        loggingBackend().log(level, flf, text.str());
    }

    std::optional<Level> LogFrontend::get_loglevel_from_env()
    {
        const char* env = getenv("LOGGING_LEVEL");

        if (env == nullptr)
        {
            return {};
        }
        else
        {
            return nr2level(std::stoul(env) & 0xff);
        }
    }

    Level LogFrontend::get_default_loglevel()
    {
        auto default_log_level = Level::info;
        const char* env = getenv("LOGGING_DEFAULT_LEVEL");

        if (env) {
            auto level = nr2level(std::stoul(env) & 0xff);
            if (level) {
                default_log_level = level.value();
            }
        }
        return default_log_level;
    }

    // ConsoleLogBackend
    ConsoleLogBackend::ConsoleLogBackend()
    {
        m_colorOut = getenv("DISABLE_LOGGING_COLORS") == nullptr;
    }

    void ConsoleLogBackend::log_p(Level level, const Flf &flf, const char* text)
    {
        std::lock_guard sl{m_mutex };

        const char* dark = "";
        const char* allOff = "";
        const char* levelColor = "";

        if (m_colorOut) {
            dark = "\33[1;30m";
            allOff = "\33[0m";
            levelColor = level2color(level);
        }

        FILE * const dst = stderr;
        fprintf(dst, "%s%-*s:%s ", levelColor, MaxLengthLevel, level2string(level), allOff);
        fprintf(dst, "%s[%s]%s ", dark, dots::type::TimePoint::Now().toString().c_str(), allOff);
        fprintf(dst, "%s", text);
        fprintf(dst, " %s(%s:%d (%s))%s\n", dark, flf.file.data(), flf.line, flf.func.data(), allOff);
        fflush(dst);
    }

    void ConsoleLogBackend::log(Level level, const Flf &flf, const std::string& text)
    {
        log_p(level, flf, text.c_str());
    }

    const char *ConsoleLogBackend::level2color(Level level)
    {
        switch(level)
        {
            case Level::data:  return "\33[37m";
            case Level::debug: return "\33[37m";
            case Level::info:  return "\33[1;34m";
            case Level::notice:return "\33[1;32m";
            case Level::warn:  return "\33[1;33m";
            case Level::error: return "\33[1;31m";
            case Level::crit:  return "\33[1;45m";
            case Level::emerg: return "\33[1;41m";
        }

        return "";
    }

    // SyslogBackend
#ifdef __unix__
    SyslogBackend::SyslogBackend(const char* ident, int option)
    {
        openlog(ident, option, LOG_USER);
    }

    SyslogBackend::~SyslogBackend()
    {
        closelog();
    }

    void SyslogBackend::log_p(Level level, const Flf &flf, const char* messagea)
    {
        std::lock_guard sl{m_mutex };
        syslog(toSyslogLevel(level), "%s:%d (%s): %s", flf.file.data(), flf.line, flf.func.data(), messagea);
    }

    void SyslogBackend::log(Level level, const Flf &flf, const std::string& text)
    {
        log_p(level, flf, text.c_str());
    }

    int SyslogBackend::toSyslogLevel(Level level)
    {
        switch(level)
        {
            case Level::data:
            case Level::debug: return LOG_DEBUG;
            case Level::info:  return LOG_INFO;
            case Level::notice:return LOG_NOTICE;
            case Level::warn:  return LOG_WARNING;
            case Level::error: return LOG_ERR;
            case Level::crit:  return LOG_CRIT;
            case Level::emerg: return LOG_EMERG;
        }
        return 0;
    }
#endif

    std::optional<Level> nr2level(uint8_t nr)
    {
        switch(nr)
        {
            case 1: return Level::data;
            case 2: return Level::debug;
            case 3: return Level::info;
            case 4: return Level::notice;
            case 5: return Level::warn;
            case 6: return Level::error;
            case 7: return Level::crit;
            case 8: return Level::emerg;
        }
        return {};
    }

    const char *level2string(Level level)
    {
        switch(level)
        {
            case Level::data:  return "data";
            case Level::debug: return "debug";
            case Level::info:  return "info";
            case Level::notice:return "notice";
            case Level::warn:  return "warn";
            case Level::error: return "error";
            case Level::crit:  return "crit";
            case Level::emerg: return "emerg";
        }
        return "";
    }

    static std::shared_ptr<LogBackend> createBackend()
    {
        const char *env = getenv("LOGGING_BACKEND");

        if (env)
        {
            const std::string name = env;

            if (name == "con")
            {
                return std::make_shared<ConsoleLogBackend>();
            }
            else if (name == "syslog")
            {
#ifdef __unix__
                return std::make_shared<SyslogBackend>();
#else
                throw std::logic_error{ "syslog is not available on this platform" };
#endif
            }
        }

        return std::make_shared<ConsoleLogBackend>();
    }

    LogFrontend& loggingFrontend()
    {
        static auto s_frontend = std::make_unique<LogFrontend>();
        return *s_frontend;
    }

    LogBackend& loggingBackend()
    {
        static std::shared_ptr<LogBackend> s_backend(createBackend());
        return g_loggingBackend ? *g_loggingBackend : *s_backend;
    }

    std::shared_ptr<LogBackend> g_loggingBackend;
}
