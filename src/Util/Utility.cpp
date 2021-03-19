#include <sstream>
#include <functional>
#include <imgui.h>
#include "Utility.h"

std::string GenerateID(const ASTNode* node, const char* ID)
{
    std::stringstream ss;
    ss << static_cast<const void*>(node);
    std::string id = ss.str();
    id.insert(0, ID);
    return id;
}

void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}