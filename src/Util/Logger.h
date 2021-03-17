#pragma once
#include <imgui.h>
#define NOGDI
#define SPDLOG_EOL ""
#include <spdlog/logger.h>
#include <spdlog/sinks/ostream_sink.h>

class Logger
{
public:
    enum class Level 
    {
        DEBUG    = spdlog::level::debug,
        INFO     = spdlog::level::info,
        WARN     = spdlog::level::warn,
        ERROR    = spdlog::level::err,
    };
private:
    struct LineInfo
    {
        int    lineNo;
        ImVec4 color;
    };
    ImGuiTextBuffer    buffer;
    ImGuiTextFilter    filter;
    ImVector<LineInfo> lineOffsets;
    bool               autoScroll;
    int                logLevel;
    ImVec4             color[4] =
    {
        {1.0f, 1.0f, 1.0f, 1.0f},  // WHITE  by default
        {0.4f, 1.0f, 0.4f, 1.0f},  // GREEN  by default
        {1.0f, 1.0f, 0.4f, 1.0f},  // YELLOW by default
        {1.0f, 0.4f, 0.4f, 1.0f}   // RED    by default
    };
    std::shared_ptr<spdlog::logger> spdlogLogger;
    std::ostringstream oss;

    Logger();
    void Log(const Level lvl, const char* fmt, ...) IM_FMTARGS(2);
public:
    static Logger* Instance();

    void SetLevel(const Level lvl) 
    { 
        logLevel = static_cast<int>(lvl) - 1;
        spdlogLogger->set_level(static_cast<spdlog::level::level_enum>(lvl));
    }

    void SetLevelColor(const Level lvl, ImVec4 col) { color[static_cast<int>(lvl) - 1] = col; }

    /*template<typename ...Args>
    static void Debug(const char* fmt, Args... args) { Instance()->Log(Level::DEBUG, fmt, args...); }
    template<typename ...Args>
    static void Info(const char*  fmt, Args... args) { Instance()->Log(Level::INFO,  fmt, args...); }
    template<typename ...Args>
    static void Warn(const char*  fmt, Args... args) { Instance()->Log(Level::WARN,  fmt, args...); }
    template<typename ...Args>
    static void Error(const char* fmt, Args... args) { Instance()->Log(Level::ERROR, fmt, args...); }*/

    template<typename T>
    static void Debug(T& msg)
    {
        auto loggerInstance = Logger::Instance();
        loggerInstance->spdlogLogger->debug(msg);
        loggerInstance->Log(Level::DEBUG, loggerInstance->oss.str().c_str());
    }

    template<typename T>
    static void Info(T& msg)
    {
        auto loggerInstance = Logger::Instance();
        loggerInstance->spdlogLogger->info(msg);
        loggerInstance->Log(Level::INFO, loggerInstance->oss.str().c_str());
    }

    template<typename T>
    static void Warn(T& msg)
    {
        auto loggerInstance = Logger::Instance();
        loggerInstance->spdlogLogger->warn(msg);
        loggerInstance->Log(Level::WARN, loggerInstance->oss.str().c_str());
    }

    template<typename T>
    static void Error(T& msg)
    {
        auto loggerInstance = Logger::Instance();
        loggerInstance->spdlogLogger->error(msg);
        loggerInstance->Log(Level::ERROR, loggerInstance->oss.str().c_str());
    }

    template<typename FormatString, typename ...Args>
    static void Debug(const FormatString& fmt, Args&& ...args)
    {
        auto loggerInstance = Logger::Instance();
        loggerInstance->spdlogLogger->debug(fmt, args...);
        loggerInstance->Log(Level::DEBUG, loggerInstance->oss.str().c_str());
    }

    template<typename FormatString, typename ...Args>
    static void Info(const FormatString& fmt, Args&& ...args)
    {
        auto loggerInstance = Logger::Instance();
        loggerInstance->spdlogLogger->info(fmt, args...);
        loggerInstance->Log(Level::INFO, loggerInstance->oss.str().c_str());
    }

    template<typename FormatString, typename ...Args>
    static void Warn(const FormatString& fmt, Args&& ...args)
    {
        auto loggerInstance = Logger::Instance();
        loggerInstance->spdlogLogger->warn(fmt, args...);
        loggerInstance->Log(Level::WARN, loggerInstance->oss.str().c_str());
    }

    template<typename FormatString, typename ...Args>
    static void Error(const FormatString& fmt, Args&& ...args)
    {
        auto loggerInstance = Logger::Instance();
        loggerInstance->spdlogLogger->error(fmt, args...);
        loggerInstance->Log(Level::ERROR, loggerInstance->oss.str().c_str());
    } 

    void Clear();
    void Draw(const char* title, bool* p_open = NULL);
};