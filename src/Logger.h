#pragma once
#include <imgui.h>

class Logger
{
private:
    ImGuiTextBuffer buffer;
    ImGuiTextFilter filter;
    ImVector<int>   lineOffsets;
    bool            autoScroll;

    Logger();
public:
    static Logger* Instance();

    void Clear();
    void Log(const char* fmt, ...) IM_FMTARGS(2);
    void Draw(const char* title, bool* p_open = NULL);
};