#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui-SFML.h>

#include <TextEditor.h>
#include <ImGuiFileDialog.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <magic_enum.hpp>

#include "Parser/Parser.h"
#include "Semantics/SemanticAnalyzer.h"
#include "AST/ASTVisualizer.h"
#include "AST/ASTPrinterJson.h"
#include "CodeGen/CodeGenerator.h"
#include "Util/Logger.h"
#include "Util/Utility.h"

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
    ASTVisualizer astViz;
    SemanticAnalyzer sem;
    CodeGenerator codeGen;

    // Registering order MATTERS - should mirror the order they should be run in
    ModuleManager::Instance()->RegisterObservers(&lexer, &parser, &sem, &codeGen);

    // Set callback for all registered modules that enables them to get back the line text from an error location
    ModuleManager::Instance()->UpdateGetSourceLineCallback([&editor](const int line) {
        const auto currentCoords = editor.GetCursorPosition();
        editor.SetCursorPosition({ line - 1, 0 });
        std::string sourceLine = editor.GetCurrentLineText();
        editor.SetCursorPosition(currentCoords);
        return sourceLine;
    });

    ASTPrinterJson jsonPrinter;
    // Pass extra stuff to the ASTVisualizer to render
    astViz.SetExtrasToRender([&jsonPrinter]() {
        ImGui::SameLine();
        static float width = 100.0f;
        float pos = width + ImGui::GetStyle().ItemSpacing.x;
        ImGui::SameLine(ImGui::GetWindowWidth() - pos);
        if (ImGui::Button("AST to JSON")) jsonPrinter.PrintAST();
        width = ImGui::GetItemRectSize().x;
    });

    // Registering order does NOT matter, parser's AST is observed for updates by many
    parser.RegisterObservers(&sem, &astViz, &jsonPrinter, &codeGen);
    // Semantic Analyzer state is observed by CodeGen - if sem analysis failed there is not point to run code gen
    sem.RegisterObservers(&codeGen);

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
                        FD::Save(&editor, filePath);
                    break;
                case sf::Keyboard::O:
                    if (event.key.control && !ImGuiFileDialog::Instance()->IsOpened())
                        ImGuiFileDialog::Instance()->OpenModal("ChooseFileKey", "Choose File", FD::saveFileFilter, FD::dialogDir, "");
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
                if (ImGui::MenuItem("New")) 
                { 
                    editor.SetText("");
                    fileName = "Untitled";
                    filePath.clear();
                    ModuleManager::Instance()->NotifyObservers(Notify::ToReset);
                }
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                    ImGuiFileDialog::Instance()->OpenModal("ChooseFileKey", "Choose File", FD::openFileFilter, FD::dialogDir, "");
                if (ImGui::MenuItem("Save", "Ctrl+S")) FD::Save(&editor, filePath);
                if (ImGui::MenuItem("Save As..")) 
                    ImGuiFileDialog::Instance()->OpenModal("SaveAsKey", "Save File As", FD::saveFileFilter, FD::dialogDir, "",
                        std::bind(&GUI::HelpMarker, FD::tip), 50.0f, 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
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
        static GUI::ActionButton buttons[4] = {
        {"CodeGen",  100.0f, [&]() { ModuleManager::Instance()->RunModulesUpTo(&codeGen); } },
        {"Semantic", 100.0f, [&]() { ModuleManager::Instance()->RunModulesUpTo(&sem);     } },
        {"Parse",    100.0f, [&]() { ModuleManager::Instance()->RunModulesUpTo(&parser);  } },
        {"Tokenize", 100.0f, [&]() { ModuleManager::Instance()->RunModulesUpTo(&lexer);   } } };
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
        if (editor.IsTextChanged()) ModuleManager::Instance()->NotifyObservers(Notify::ShouldRun);
        editor.Render("TextEditor");
        ImGui::End();

        static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable;
        ImGui::Begin("Lexer Output");
        if (ImGui::BeginTable("Tokens", 3, flags))
        {
            // To freeze the first row of the table we need the ImGuiTableFlags_ScrollY flag
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Token");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Line:Column");
            ImGui::TableHeadersRow();
            const auto& tokens = lexer.GetTokens();
            for (int row = 0; row < tokens.size(); row++)
            {
                ImGui::TableNextRow();
                const auto& [str, coords, type] = tokens.at(row);
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", str.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", magic_enum::enum_name(type).data());
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%d:%d", coords.line, coords.col);
            }
            ImGui::EndTable();
        }
        ImGui::End();

        // Render the parser output window, if there is an AST
        astViz.RenderAST();

        // Semantics Window
        int open_action = -1;
        if (ImGui::Begin("Semantic Analysis"))
        {
            if (sem.CanRender())
            {
                if (ImGui::Button("Expand Scopes"))   open_action = 1;
                ImGui::SameLine();
                if (ImGui::Button("Collapse Scopes")) open_action = 0;
                ImGui::Separator();
            }

            static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
            if (ImGui::BeginTable("SemInfo", 3, flags))
            {
                static const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
                // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
                ImGui::TableSetupColumn("Symbol", ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 12.0f);
                ImGui::TableSetupColumn("Nested Level", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
                ImGui::TableHeadersRow();

                sem.Render(open_action);

                ImGui::EndTable();
            }
        }
        ImGui::End();

        // Code Gen Window (TAC & 'Assembly')
        if (ImGui::Begin("Code Generation"))
        {
            if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
            {
                ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
                if (ImGui::BeginTabItem("TAC"))
                {
                    static std::string tac;
                    tac = codeGen.GetTAC();
                    ImGui::InputTextMultiline("##tac", &tac, ImVec2(-1, -1), ImGuiInputTextFlags_ReadOnly);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Assembly"))
                {
                    static std::string x86;
                    x86 = codeGen.Getx86();
                    ImGui::InputTextMultiline("##x86", &x86, ImVec2(-1, -1), ImGuiInputTextFlags_ReadOnly);
                    ImGui::EndTabItem();
                }
                ImGui::PopStyleColor();
                ImGui::EndTabBar();
            }
        }
        ImGui::End();

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
                if (FD::PrintToFile(&editor, fpName))
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

/*  TODO:
*     - Docking - SFML backend does not support it, atm can only be done by switching backends
*
*     - Horizontal scrollbar for tab panes
*     - No copy of string with GetTAC/Getx86?
*     - Tidy up/Improve/split CodeGen
*/