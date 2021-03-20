#include <sstream>
#include <fstream>
#include <functional>
#include <imgui.h>
#include <TextEditor.h>
#include <ImGuiFileDialog.h>

#include "Utility.h"
#include "../AST/ASTPrinterJson.h"

// General Helpers
namespace Util
{
    std::string GenerateID(const ASTNode* node, const char* ID)
    {
        std::stringstream ss;
        ss << static_cast<const void*>(node);
        std::string id = ss.str();
        id.insert(0, ID);
        return id;
    }
}

// GUI Helpers
namespace GUI
{
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

    void ShowJsonButton(ASTNode* AST)
    {
        if (!AST) return;
        if (ImGui::Begin("Parser Output"))
        {
            // GHETTO SOLUTION to append extras in the parser window.. (before nodes)
            if (ImGui::BeginChild("ParserExtra"))
            {
                static float width = 100.0f;
                float pos = width + ImGui::GetStyle().ItemSpacing.x;
                ImGui::SameLine(ImGui::GetWindowWidth() - pos);
                if (ImGui::Button("AST to JSON"))
                {
                    // This will happen even if the text has not changed...
                    ASTPrinterJson jsonPrinter;
                    jsonPrinter.PrintAST(*AST);
                }
                width = ImGui::GetItemRectSize().x;
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }
}

// FileDialog Helpers
namespace FD
{
    [[maybe_unused]] bool PrintToFile(const TextEditor* editor, const std::string& filePath)
    {
        std::ofstream outfile;
        outfile.open(filePath);
        if (outfile)
        {
            outfile << std::noskipws;
            outfile << editor->GetText();
            outfile.close();
            return true;
        }
        else ImGui::OpenPopup("Error");
        return false;
    }

    void Save(const TextEditor* editor, const std::string& filePath)
    {
        if (filePath.empty())
            ImGuiFileDialog::Instance()->OpenModal("SaveAsKey", "Save File As", saveFileFilter, dialogDir,
                "", std::bind(&GUI::HelpMarker, tip), 30.0f, 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
        else PrintToFile(editor, filePath);
    }
}