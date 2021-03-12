#pragma once
#include <imgui.h>

class Logger
{
private:
    ImGuiTextBuffer buffer;
    ImGuiTextFilter filter;
    ImVector<int>   lineOffsets;
    bool            autoScroll;
    int             logLevel;

    Logger();
public:
    static Logger* Instance();

    enum class Level { DEBUG, INFO, WARN, ERROR, CRITICAL };

    void Clear();
    void Log(const Level lvl, const char* fmt, ...) IM_FMTARGS(2);
    void Draw(const char* title, bool* p_open = NULL);

    void SetLevel(const Level lvl) { logLevel = static_cast<int>(lvl); }
};