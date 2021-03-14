#pragma once
#include <imgui.h>

class Logger
{
public:
    enum class Level { DEBUG, INFO, WARN, ERROR, CRITICAL };
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

    Logger();
    void Log(const Level lvl, const char* fmt, ...) IM_FMTARGS(2);
public:
    static Logger* Instance();

    void SetLevel(const Level lvl) { logLevel = static_cast<int>(lvl); }
    template <typename L>
    void SetLevelColor(const L lvl, ImVec4 col)
    {
        if (lvl < Level::DEBUG || lvl > Level::ERROR) return;
        color[static_cast<int>(lvl)] = col;
    }

    template<typename ...Args>
    static void Debug(const char* fmt, Args... args) { Instance()->Log(Level::DEBUG, fmt, args...); }
    template<typename ...Args>
    static void Info(const char*  fmt, Args... args) { Instance()->Log(Level::INFO,  fmt, args...); }
    template<typename ...Args>
    static void Warn(const char*  fmt, Args... args) { Instance()->Log(Level::WARN,  fmt, args...); }
    template<typename ...Args>
    static void Error(const char* fmt, Args... args) { Instance()->Log(Level::ERROR, fmt, args...); }

    void Clear();
    void Draw(const char* title, bool* p_open = NULL);
};