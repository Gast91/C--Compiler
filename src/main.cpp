#include <imgui.h>
#include <imgui-SFML.h>

#include <TextEditor.h>
#include <ImGuiFileDialog.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <magic_enum.hpp>

#include <fstream>

#include "Parser/Parser.h"
#include "AST/ASTVisualizer.h"
#include "AST/ASTPrinterJson.h"

const char* saveFileFilter = ".h,.hpp,.cpp,.txt";
const char* openFileFilter = "Source files (*.cpp *.h *.hpp *.txt){.cpp,.h,.hpp,.txt}";
const char* tip = "Don't forget to select the type you want to save your file as!";
const char* dialogDir = "D:/Desktop/CTests"; // TEMP! - USER SPECIFIC


static void HelpMarker(const char* desc) // move somewhere else/remove/whatever
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

struct Button
{
    const char* label;
    float width;
    std::function<void()> action;
};

[[maybe_unused]] static bool PrintToFile(const TextEditor& editor, const std::string& filePath)
{
    std::ofstream outfile;
    outfile.open(filePath);
    if (outfile)
    {
        outfile << std::noskipws;
        outfile << editor.GetText();
        outfile.close();
        return true;
    }
    else ImGui::OpenPopup("Error");
    return false;
}

static void Save(const TextEditor& editor, const std::string& filePath)
{
    if (filePath.empty())
        ImGuiFileDialog::Instance()->OpenModal("SaveAsKey", "Save File As", saveFileFilter, dialogDir,
            "", std::bind(&HelpMarker, tip), 30.0f, 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
    else PrintToFile(editor, filePath);
}

void ShowJsonButton(ASTNode* AST)
{
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

int main()
{
    sf::RenderWindow window(sf::VideoMode().getDesktopMode(), "EditorTest");
    window.setVerticalSyncEnabled(true);

    ImGui::SFML::Init(window);  

    // Since we only draw ImGui stuff atm we need this
    window.resetGLStates();
    sf::Clock deltaClock;

    TextEditor editor;
    editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
    editor.SetShowWhitespaces(false);
    std::string filePath;
    std::string fileName = "Untitled";

    ImGuiFileDialog::Instance()->SetExtentionInfos(".cpp", ImVec4(1.0f, 1.0f, 0.0f, 0.9f));
    ImGuiFileDialog::Instance()->SetExtentionInfos(".h",   ImVec4(0.0f, 1.0f, 0.0f, 0.9f));
    ImGuiFileDialog::Instance()->SetExtentionInfos(".hpp", ImVec4(0.0f, 0.0f, 1.0f, 0.9f));
    ImGuiFileDialog::Instance()->SetExtentionInfos(".txt", ImVec4(1.0f, 0.0f, 1.0f, 0.9f));

    Lexer lexer(&editor);
    Parser parser(&lexer);
    // Registering order MATTERS - should mirror the order they should be run in
    ModuleManager::Instance()->RegisterObservers(&lexer, &parser);

    ASTVisualizer viz;
    while (window.isOpen()) 
    {
        sf::Event event;
        while (window.pollEvent(event)) 
        {
            ImGui::SFML::ProcessEvent(event);
            switch (event.type)
            {
            case sf::Event::Closed: window.close(); break;
            case sf::Event::KeyPressed:
                switch (event.key.code)
                {
                case sf::Keyboard::S:
                    if (event.key.control && !ImGuiFileDialog::Instance()->IsOpened())
                        Save(editor, filePath);
                    break;
                case sf::Keyboard::O:
                    if (event.key.control && !ImGuiFileDialog::Instance()->IsOpened())
                        ImGuiFileDialog::Instance()->OpenModal("ChooseFileKey", "Choose File", saveFileFilter, "D:/Desktop/CTests", "");
                    break;
                case sf::Keyboard::Escape: window.close(); break;
                default: break;
                }
                break;
            default: break;
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
        ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New")) { editor.SetText(""); fileName = "Untitled"; filePath.clear(); }
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                    ImGuiFileDialog::Instance()->OpenModal("ChooseFileKey", "Choose File", openFileFilter, dialogDir, "");
                if (ImGui::MenuItem("Save", "Ctrl+S")) Save(editor, filePath);
                if (ImGui::MenuItem("Save As..")) 
                    ImGuiFileDialog::Instance()->OpenModal("SaveAsKey", "Save File As", saveFileFilter, dialogDir, "",
                        std::bind(&HelpMarker, tip), 50.0f, 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
                if (ImGui::MenuItem("Quit", "Alt-F4")) window.close();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                bool ro = editor.IsReadOnly();
                if (ImGui::Checkbox("Read-only mode", &ro))                                                    editor.SetReadOnly(ro);
                ImGui::Separator();

                if (ImGui::MenuItem("Undo",   "ALT-Backspace", nullptr, !ro && editor.CanUndo()))              editor.Undo();
                if (ImGui::MenuItem("Redo",   "Ctrl-Y",        nullptr, !ro && editor.CanRedo()))              editor.Redo();
                ImGui::Separator();

                if (ImGui::MenuItem("Copy",   "Ctrl-C", nullptr,        editor.HasSelection()))                editor.Copy();
                if (ImGui::MenuItem("Cut",    "Ctrl-X", nullptr, !ro && editor.HasSelection()))                editor.Cut();
                if (ImGui::MenuItem("Delete", "Del",    nullptr, !ro && editor.HasSelection()))                editor.Delete();
                if (ImGui::MenuItem("Paste",  "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr)) editor.Paste();
                ImGui::Separator();

                if (ImGui::MenuItem("Select all", "Ctrl-A", nullptr)) 
                    editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View"))
            {
                bool ws = editor.IsShowingWhitespaces();
                if (ImGui::Checkbox("Show whitespaces", &ws)) editor.SetShowWhitespaces(ws);
                if (ImGui::MenuItem("Dark palette"))          editor.SetPalette(TextEditor::GetDarkPalette());
                if (ImGui::MenuItem("Light palette"))         editor.SetPalette(TextEditor::GetLightPalette());
                if (ImGui::MenuItem("Retro blue palette"))    editor.SetPalette(TextEditor::GetRetroBluePalette());
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Text Editor Status Bar - looong name will overlap with buttons
        const auto cpos = editor.GetCursorPosition();
        ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
            editor.IsOverwrite() ? "Ovr" : "Ins",
            editor.CanUndo() ? "*" : " ",
            editor.GetLanguageDefinition().mName.c_str(), fileName.c_str());

        // Right Aligned Module Button Group
        static Button buttons[4] = {
        {"CodeGen",  100.0f, []()  {/*ModuleManager::Instance()->RunModulesUpTo(&codeGen);*/} },
        {"Semantic", 100.0f, []()  {/*ModuleManager::Instance()->RunModulesUpTo(&sem);*/    } },
        {"Parse",    100.0f, [&]() { ModuleManager::Instance()->RunModulesUpTo(&parser);    } },
        {"Tokenize", 100.0f, [&]() { ModuleManager::Instance()->RunModulesUpTo(&lexer);     } } };
        const float itemSpacing = ImGui::GetStyle().ItemSpacing.x;
        float pos = 0.0f;
        for (int i = 0; i < 4; ++i)
        {
            pos += buttons[i].width + itemSpacing;
            ImGui::SameLine(ImGui::GetWindowWidth() - pos);
            if (ImGui::SmallButton(buttons[i].label)) buttons[i].action();
            buttons[i].width = ImGui::GetItemRectSize().x;
        }
        ImGui::Separator();
        ImGui::Spacing();
        
        // Module notification HAS to happen before the editor is rendered since changed flag is reset
        if (editor.IsTextChanged()) ModuleManager::Instance()->NotifyModulesToRun();
        editor.Render("TextEditor");
        ImGui::End();

        static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable;
        ImGui::Begin("Lexer Output");
        if (ImGui::BeginTable("Tokens", 3, flags))
        {
            // Freeze first row of the table - NEEDS ImGuiTableFlags_ScrollY flag
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Token");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Line:Column");
            ImGui::TableHeadersRow();
            const auto& tokens = lexer.GetTokens();
            for (int row = 0; row < tokens.size(); row++)
            {
                ImGui::TableNextRow();
                const auto& [str, type, line, col] = tokens.at(row);
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", str.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", magic_enum::enum_name(type).data());
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%d:%d", line, col);
            }
            ImGui::EndTable();
        }
        ImGui::End();

        // Parser Output Window + AST Tree (if available)
        auto AST = parser.GetAST();
        viz.RenderAST(*AST);
        if (AST) ShowJsonButton(AST);      

        if (ImGuiFileDialog::Instance()->Display("ChooseFileKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::ifstream infile;
                filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                infile.open(filePath);
                if (infile)
                {
                    std::vector<std::string> lines;
                    std::string line;
                    while (std::getline(infile, line)) lines.push_back(line);
                    editor.SetTextLines(lines);
                    fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
                    infile.close();
                }
                else ImGui::OpenPopup("Error");
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (ImGuiFileDialog::Instance()->Display("SaveAsKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                const std::string fpName = ImGuiFileDialog::Instance()->GetFilePathName();
                if (PrintToFile(editor, fpName))
                {
                    filePath = fpName;
                    fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_NoResize))
        {
            ImGui::Text("Cannot read/write file");
            ImGui::Separator();
            if (ImGui::Button("OK", ImVec2(ImGui::GetContentRegionAvail().x, 0))) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        Logger::Instance()->Draw("Console Output", NULL);
        //ImGui::ShowDemoWindow();

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }
    ImGui::SFML::Shutdown();
}

/*  TODO: NO SEMANTICS UNTIL CLEANUP AND MERGE WITH C--Compiler project
*     - Docking - SFML backend issues                                        - low priority - unachievable atm (switch to other backend?)
*     - Merge all compiler branches, create new on (dissertation)            - high priority
*         Master becomes an ImGui frontend - can I carry commit history?                                  
*     - spdlog? multisink for file and/or 'console' output?                  - medium priority - decide - this changes all prints to file(dialogs etc)
*/