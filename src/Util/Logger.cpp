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
    lineOffsets.push_back({ 0, color[static_cast<int>(Level::DEBUG)] });
}

void Logger::Log(const Level lvl, const char* fmt, ...) IM_FMTARGS(2)
{
    if (static_cast<int>(lvl) < logLevel || lvl > Level::ERROR) return;

    // Each line MUST be its own thing, else colors get scrambled FIX-ME? 
    lineOffsets.back().color = color[static_cast<int>(lvl)];

    int old_size = buffer.size();
    va_list args;
    va_start(args, fmt);
    buffer.appendfv(fmt, args);
    va_end(args);
    for (int new_size = buffer.size(); old_size < new_size; old_size++)
    {
        if (buffer[old_size] == '\n')
            lineOffsets.push_back({ old_size + 1, color[static_cast<int>(lvl)] });
    }
}

void Logger::Draw(const char* title, bool* p_open)
{
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(title, p_open, ImGuiWindowFlags_MenuBar)) { ImGui::End(); return; }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Settings"))
        {
            ImGui::MenuItem("Auto-scroll", NULL, &autoScroll);
            ImGui::Separator();
            if (ImGui::BeginMenu("Logging Levels"))
            {
                if (ImGui::BeginMenu("Colors"))
                {
                    ImGui::ColorEdit4("Debug", (float*)&color[0], ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit4("Info",  (float*)&color[1], ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit4("Warn",  (float*)&color[2], ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit4("Error", (float*)&color[3], ImGuiColorEditFlags_NoInputs);
                    ImGui::EndMenu();
                }
                ImGui::RadioButton("Debug", &logLevel, 0);
                ImGui::RadioButton("Info",  &logLevel, 1);
                ImGui::RadioButton("Warn",  &logLevel, 2);
                ImGui::RadioButton("Error", &logLevel, 3);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Copy"))  ImGui::LogToClipboard();
            if (ImGui::MenuItem("Clear")) Clear();
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    filter.Draw("Filter", -100.0f);

    ImGui::Separator();
    ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);  

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    const char* buf = buffer.begin();
    const char* buf_end = buffer.end();
    if (filter.IsActive())
    {
        for (int line_no = 0; line_no < lineOffsets.size(); line_no++)
        {
            const char* line_start = buf + lineOffsets[line_no].lineNo;
            const char* line_end = (line_no + 1 < lineOffsets.size()) ? (buf + lineOffsets[line_no + 1].lineNo - 1) : buf_end;
            if (filter.PassFilter(line_start, line_end))
            {
                ImGui::PushStyleColor(ImGuiCol_Text, lineOffsets[line_no].color);
                //ImGui::PushTextWrapPos(ImGui::GetFontSize() * 50.0f);
                ImGui::TextUnformatted(line_start, line_end);
                //ImGui::PopTextWrapPos();
                ImGui::PopStyleColor();
            }
        }
    }
    else
    {
        ImGuiListClipper clipper;
        clipper.Begin(lineOffsets.size());
        while (clipper.Step())
        {
            for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
            {
                const char* line_start = buf + lineOffsets[line_no].lineNo;
                const char* line_end = (line_no + 1 < lineOffsets.size()) ? (buf + lineOffsets[line_no + 1].lineNo - 1) : buf_end;
                ImGui::PushStyleColor(ImGuiCol_Text, lineOffsets[line_no].color);
                //ImGui::PushTextWrapPos(ImGui::GetFontSize() * 50.0f);
                ImGui::TextUnformatted(line_start, line_end);
                //ImGui::PopTextWrapPos();
                ImGui::PopStyleColor();
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