#include <imgui.h>
#include <imgui-SFML.h>

#include <TextEditor.h>
#include <ImGuiFileDialog.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <magic_enum.hpp>

#include <fstream>

#include "Logger.h"
#include "Lexer.h"
#include "Parser.h"
#include "ASTVisualizer.h"

constexpr const char* fileTypeFilter = "Source files (*.cpp *.h *.hpp *.txt){.cpp,.h,.hpp,.txt}";

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

static void ShowLogger(bool* p_open)
{
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Console Output", p_open);

    /* Can add other stuff here */

    ImGui::End();

    // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
    Logger::Instance()->Draw("Console Output", p_open);
}

static void ShowSaveDialog(const TextEditor& editor, const std::string& fileName)
{
    if (fileName == "Untitled")
        ImGuiFileDialog::Instance()->OpenModal("SaveAsKey", "Save File As", fileTypeFilter, "D:/Desktop/CTests", "", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
    else
    {
        std::ofstream outfile;
        outfile.open(fileName);
        if (outfile)
        {
            outfile << std::noskipws;
            outfile << editor.GetText();
            outfile.close();
        }
        else ImGui::OpenPopup("Error");
    }
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
    std::string fileName = "Untitled";

    ImGuiFileDialog::Instance()->SetExtentionInfos(".cpp", ImVec4(1.0f, 1.0f, 0.0f, 0.9f));
    ImGuiFileDialog::Instance()->SetExtentionInfos(".h",   ImVec4(0.0f, 1.0f, 0.0f, 0.9f));
    ImGuiFileDialog::Instance()->SetExtentionInfos(".hpp", ImVec4(0.0f, 0.0f, 1.0f, 0.9f));
    ImGuiFileDialog::Instance()->SetExtentionInfos(".txt", ImVec4(1.0f, 0.0f, 1.0f, 0.9f));

    auto getLineAt = [&editor](const int line, const int col)
    {
        const auto currentCoords = editor.GetCursorPosition();
        editor.SetCursorPosition({ line, col });
        std::string sourceLine = editor.GetCurrentLineText();
        editor.SetCursorPosition(currentCoords);
        return sourceLine;
    };
    Lexer lexer(getLineAt);
    Parser parser(&lexer);
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
                /*case sf::Keyboard::LControl: 
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) && !ImGui::IsPopupOpen("Error") && !ImGuiFileDialog::Instance()->IsOpened())
                        ShowSaveDialog(editor, fileName);
                    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::O) && !ImGui::IsPopupOpen("Error") && !ImGuiFileDialog::Instance()->IsOpened())
                        ImGuiFileDialog::Instance()->OpenModal("ChooseFileKey", "Choose File", fileTypeFilter, "D:/Desktop/CTests", "");
                    break;*/
                case sf::Keyboard::Escape: window.close(); break;
                default: break;
                }
                break;
            default: break;
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        auto cpos = editor.GetCursorPosition();
        ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
        ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New"))
                    editor.SetText(""); fileName = "Untitled";  //?? handle file name and extension seperately?
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                    ImGuiFileDialog::Instance()->OpenModal("ChooseFileKey", "Choose File", fileTypeFilter, "D:/Desktop/CTests", "");
                if (ImGui::MenuItem("Save", "Ctrl+S")) ShowSaveDialog(editor, fileName);
                if (ImGui::MenuItem("Save As..")) 
                    ImGuiFileDialog::Instance()->OpenModal("SaveAsKey", "Save File As", fileTypeFilter, "D:/Desktop/CTests", "", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
                if (ImGui::MenuItem("Quit", "Alt-F4")) window.close();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                bool ro = editor.IsReadOnly();
                if (ImGui::Checkbox("Read-only mode", &ro))                                                  editor.SetReadOnly(ro);
                ImGui::Separator();

                if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))               editor.Undo();
                if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))                      editor.Redo();
                ImGui::Separator();

                if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))                        editor.Copy();
                if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))                  editor.Cut();
                if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))                  editor.Delete();
                if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr)) editor.Paste();
                ImGui::Separator();

                if (ImGui::MenuItem("Select all", nullptr, nullptr)) 
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

        ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
            editor.IsOverwrite() ? "Ovr" : "Ins",
            editor.CanUndo() ? "*" : " ",
            editor.GetLanguageDefinition().mName.c_str(), fileName.c_str());


        const float ItemSpacing = ImGui::GetStyle().ItemSpacing.x;

        // The first frame we make a guess, for subsequent frames we get the actual value
        static float CGButtonWidth = 100.0f;
        float pos = CGButtonWidth + ItemSpacing;
        ImGui::SameLine(ImGui::GetWindowWidth() - pos);
        if (ImGui::SmallButton("Code Gen"))
        {
            // if there was a change in the editor
            // run all previous steps and update all
            // windows up to Code Gen
        }
        CGButtonWidth = ImGui::GetItemRectSize().x;

        static float TACButtonWidth = 100.0f;
        pos += TACButtonWidth + ItemSpacing;
        ImGui::SameLine(ImGui::GetWindowWidth() - pos);
        if (ImGui::SmallButton("TAC"))
        {
            // if there was a change in the editor
            // run all previous steps and update all
            // windows up to TAC
        }
        TACButtonWidth = ImGui::GetItemRectSize().x;

        static float SAButtonWidth = 100.0f;
        pos += SAButtonWidth + ItemSpacing;
        ImGui::SameLine(ImGui::GetWindowWidth() - pos);
        if (ImGui::SmallButton("Semantic Analysis"))
        {
            // if there was a change in the editor
            // run all previous steps and update all
            // windows up to Semantic Analysis
        }
        SAButtonWidth = ImGui::GetItemRectSize().x;

        static float ParseButtonWidth = 100.0f;
        pos += ParseButtonWidth + ItemSpacing;
        ImGui::SameLine(ImGui::GetWindowWidth() - pos);
        if (ImGui::SmallButton("Parse"))
        {
            // if there was a change in the editor
            // run all previous steps and update all
            // windows up to AST
            parser.Parse();
        }
        ParseButtonWidth = ImGui::GetItemRectSize().x;
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("%s", "Generate AST");
            ImGui::EndTooltip();
        }

        static float TokButtonWidth = 100.0f;
        pos += TokButtonWidth + ItemSpacing;
        ImGui::SameLine(ImGui::GetWindowWidth() - pos);
        if (ImGui::SmallButton("Tokenize"))
        {
            // only if there was a change in the editor
            lexer.Tokenize(editor.GetTextLines());
            Logger::Instance()->Log("Tokenized Input (Split by whitespace):\n");
            for (const auto& toks : lexer.GetTokens()) Logger::Instance()->Log("%s ", std::get<0>(toks).c_str());
            Logger::Instance()->Log("%c", '\n');
        }
        TokButtonWidth = ImGui::GetItemRectSize().x;       

        ImGui::Separator();
        ImGui::Spacing();
        
        editor.Render("TextEditor");  
        ImGui::End();

        // MAKE THIS PART OF THE LEXER.PRINT????
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

        // Each node recursively builds the ImGui Tree through successive visits of the ASTVisualizer
        ImGui::Begin("Parser Output");
        if (const auto& AST = parser.GetAST(); AST)
            viz.RenderAST(*AST);
        ImGui::End();

        if (ImGuiFileDialog::Instance()->Display("ChooseFileKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::ifstream infile;
                fileName = ImGuiFileDialog::Instance()->GetFilePathName(); // GetSelectionDifferences?
                infile.open(fileName);
                // pop up for failure? if (!infile) { //stuff }
                if (infile)
                {
                    std::vector<std::string> lines;
                    std::string line;
                    while (std::getline(infile, line)) lines.push_back(line);
                    editor.SetTextLines(lines);

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
                std::string fpName = ImGuiFileDialog::Instance()->GetFilePathName();
                std::ofstream outfile;
                outfile.open(fpName);
                if (outfile)
                {
                    outfile << std::noskipws;
                    outfile << editor.GetText();
                    fileName = fpName;
                    outfile.close();
                }
                else ImGui::OpenPopup("Error");
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

        static bool* p_open;
        ShowLogger(p_open);
        //ImGui::ShowDemoWindow();

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }
    ImGui::SFML::Shutdown();
}

/*  TODO: NO SEMANTICS UNTIL CLEANUP AND MERGE WITH C--Compiler project
*     - Shortcut implementation  - PARTIAL/WRONG                             - low priority
*     - Cleanup/Separation of display etc                                    - high priority
*     - Docking - SFML backend issues                                        - low priority - unachievable atm (switch to other backend?)
*     - Merge all compiler branches, create new on (dissertation)            - hight priority
*         Master becomes an ImGui frontend                                   
*     - Logger checkbox for levels (verbose etc)                             - ?? priority - figure out spdlog
*         Use it through spdlog (multisink - file and/or 'console')          
*     - Utility is useless atm - move to ASTJson or something?               - ?? priority - figure out spdlog
*         ASTVisualizer with old functionality - no console output thought?  
*     - Correct Resetting of Lexer and Parser - no } crashes on second parse - medium priority
*/