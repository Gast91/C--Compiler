#include "Logger.h"

Logger::Logger()
{
    autoScroll = true;
    logLevel = 1;
    Clear();
}

Logger* Logger::Instance()
{
    static Logger loggerInstance;
    return &loggerInstance;
}

void Logger::Clear()
{
    buffer.clear();
    lineOffsets.clear();
    lineOffsets.push_back(0);
}

void Logger::Log(const Level lvl, const char* fmt, ...) IM_FMTARGS(2)
{
    if (static_cast<int>(lvl) < logLevel) return;

    int old_size = buffer.size();
    va_list args;
    va_start(args, fmt);
    buffer.appendfv(fmt, args);
    va_end(args);
    for (int new_size = buffer.size(); old_size < new_size; old_size++)
    {
        if (buffer[old_size] == '\n')
            lineOffsets.push_back(old_size + 1);
    }
}

void Logger::Draw(const char* title, bool* p_open)
{
    if (!ImGui::Begin(title, p_open))
    {
        ImGui::End();
        return;
    }

    // Options menu
    if (ImGui::BeginPopup("Options"))
    {
        ImGui::Checkbox("Auto-scroll", &autoScroll);
        ImGui::RadioButton("Debug",    &logLevel, 0);
        ImGui::RadioButton("Info",     &logLevel, 1);
        ImGui::RadioButton("Warn",     &logLevel, 2);
        ImGui::RadioButton("Error",    &logLevel, 3);
        ImGui::RadioButton("Critical", &logLevel, 4);
        ImGui::EndPopup();
    }

    // Main window
    if (ImGui::Button("Options"))
        ImGui::OpenPopup("Options");
    ImGui::SameLine();
    bool clear = ImGui::Button("Clear");
    ImGui::SameLine();
    bool copy = ImGui::Button("Copy");
    ImGui::SameLine();
    filter.Draw("Filter", -100.0f);

    ImGui::Separator();
    ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    if (clear) Clear();
    if (copy)  ImGui::LogToClipboard();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    const char* buf = buffer.begin();
    const char* buf_end = buffer.end();
    if (filter.IsActive())
    {
        for (int line_no = 0; line_no < lineOffsets.Size; line_no++)
        {
            const char* line_start = buf + lineOffsets[line_no];
            const char* line_end = (line_no + 1 < lineOffsets.Size) ? (buf + lineOffsets[line_no + 1] - 1) : buf_end;
            if (filter.PassFilter(line_start, line_end))
                ImGui::TextUnformatted(line_start, line_end);   // something else? for format and wrap
        }
    }
    else
    {
        ImGuiListClipper clipper;
        clipper.Begin(lineOffsets.Size);
        while (clipper.Step())
        {
            for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
            {
                const char* line_start = buf + lineOffsets[line_no];
                const char* line_end = (line_no + 1 < lineOffsets.Size) ? (buf + lineOffsets[line_no + 1] - 1) : buf_end;
                ImGui::TextUnformatted(line_start, line_end);    // something else? for format and wrap
            }
        }
        clipper.End();
    }
    ImGui::PopStyleVar();

    if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
}
