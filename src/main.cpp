#include <imgui.h>
#include <imgui-SFML.h>
#include <imgui_stdlib.h>
#include <TextEditor.h>
#include <imfilebrowser.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <iostream>
#include <fstream>

enum Menu
{
    NoSel  = 0,
    New    = 1 << 0,
    Open   = 1 << 1,
    Save   = 1 << 2,
    SaveAs = 1 << 3,
    Quit   = 1 << 4
};

inline Menu operator|(Menu a, Menu b)
{
    return static_cast<Menu>(static_cast<int>(a) | static_cast<int>(b));
}
inline Menu operator&(Menu a, Menu b)
{
    return static_cast<Menu>(static_cast<int>(a) & static_cast<int>(b));
}
inline Menu operator^(Menu a, Menu b)
{
    return static_cast<Menu>(static_cast<int>(a) ^ static_cast<int>(b));
}

class CodeEditor
{
private:
    std::string buffer;
    std::string title = "untitled";
    int lineCount = 1;
    ImGuiWindowFlags window_flags = 0;
    bool* p_open = nullptr;
    Menu menuOption = Menu::NoSel;
    bool loaded = false;

    void ShowMenu()
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New"))
                {
                    buffer.clear();
                    menuOption = NoSel | Menu::New;
                }
                if (ImGui::MenuItem("Open", "Ctrl+O")) { menuOption = NoSel | Menu::Open; }
                if (ImGui::MenuItem("Save", "Ctrl+S")) { menuOption = NoSel | Menu::Save; }
                if (ImGui::MenuItem("Save As.."))      { menuOption = NoSel | Menu::SaveAs; }

                ImGui::Separator();
                if (ImGui::MenuItem("Quit", "Alt+F4")) { menuOption = NoSel | Menu::Quit; }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }
    void ShowBanner()
    {
        // File name (left-aligned)
        ImGui::TextWrapped(title.c_str());
        const float ItemSpacing = ImGui::GetStyle().ItemSpacing.x;

        // Total line number (right-aligned)
        static float linesCountTextWidth = 100.0f;        // First frame guess
        float pos = linesCountTextWidth + ItemSpacing;
        ImGui::SameLine(ImGui::GetWindowWidth() - pos);
        ImGui::TextWrapped(std::to_string(lineCount).c_str());
        linesCountTextWidth = ImGui::GetItemRectSize().x; // Actual width

        // Line label (right-aligned)
        static float linesTextWidth = 100.0f;             // First frame guess
        pos += linesTextWidth + ItemSpacing;
        ImGui::SameLine(ImGui::GetWindowWidth() - pos);
        ImGui::TextWrapped("Lines: ");
        linesTextWidth = ImGui::GetItemRectSize().x;      // Actual width

        ImGui::Separator();
    }
public:
    CodeEditor()
    {
        window_flags |= ImGuiWindowFlags_MenuBar;
    }
    bool Render()
    {
        bool shouldQuit = false;
        Menu previousOpt = menuOption;
        ImGui::Begin("Code Editor", p_open, window_flags);
        ShowMenu();

        if (menuOption & Menu::NoSel)  menuOption = previousOpt;
        if (menuOption & Menu::New)
        {
            ShowBanner();  // Show file name and total line count labels
            ImGui::InputTextMultiline("##Code Editor", &buffer, { ImGui::GetWindowWidth(), ImGui::GetWindowHeight() - ImGui::GetWindowPos().y });
            loaded = true;
        }
        else if (menuOption & Menu::Open)   ImGui::Text("Open Clicked");
        else if (menuOption & Menu::Save)   ImGui::Text("Save Clicked");
        else if (menuOption & Menu::SaveAs) ImGui::Text("SaveAs Clicked");
        else if (menuOption & Menu::Quit)   shouldQuit = true;

        if (loaded)
        {
            if (ImGui::SmallButton("Tokenize"))
            {
                std::cout << "Toks!\n";
            }
        }

        ImGui::End();
        return !shouldQuit;
    }
};

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "EditorTest");
    window.setVerticalSyncEnabled(true);
    ImGui::SFML::Init(window);

    window.resetGLStates(); // call it if you only draw ImGui. Otherwise not needed.
    sf::Clock deltaClock;

    //CodeEditor codeEditor;
    TextEditor editor;
    editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
    editor.SetShowWhitespaces(false);
    std::string fileName = "Untitled.cpp";

    ImGui::FileBrowser fileDialog;
    ImGui::FileBrowser fileDialog1(ImGuiFileBrowserFlags_EnterNewFilename);
    fileDialog1.SetTitle("Save As");
    fileDialog1.SetTypeFilters({ ".h", ".cpp", ".txt" });
    // (optional) set browser properties
    fileDialog.SetTitle("Choose File");
    fileDialog.SetTypeFilters({ ".h", ".cpp", ".txt" });
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
                case sf::Keyboard::Escape: window.close(); break;
                default: break;
                }
            default: break;
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        auto cpos = editor.GetCursorPosition();
        ImGui::Begin("Text Editor Demo", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar /*| ImGuiWindowFlags_NoMove*/);  // change for docking
        ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New"))            { editor.SetText(""); fileName = "Untitled"; }  //??
                if (ImGui::MenuItem("Open", "Ctrl+O")) { fileDialog.Open();  }
                if (ImGui::MenuItem("Save"))
                {
                    auto textToSave = editor.GetText();
                    /// save text.... fileName is the path
                    // if fileName is untitled - you call SaveAs
                }
                if (ImGui::MenuItem("Save As.."))
                {
                    fileDialog1.Open();
                    // how to get the text?
                    // call the save text as above but ALSO change its name
                    // fileName = ....
                }
                
                if (ImGui::MenuItem("Quit", "Alt-F4")) window.close();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                bool ro = editor.IsReadOnly();
                if (ImGui::MenuItem("Read-only mode", nullptr, &ro))                                          editor.SetReadOnly(ro);
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
                if (ImGui::MenuItem("Dark palette"))       editor.SetPalette(TextEditor::GetDarkPalette());
                if (ImGui::MenuItem("Light palette"))      editor.SetPalette(TextEditor::GetLightPalette());
                if (ImGui::MenuItem("Retro blue palette")) editor.SetPalette(TextEditor::GetRetroBluePalette());
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
            editor.IsOverwrite() ? "Ovr" : "Ins",
            editor.CanUndo() ? "*" : " ",
            editor.GetLanguageDefinition().mName.c_str(), fileName.c_str());

        ImGui::Separator();
        ImGui::Spacing();

        editor.Render("TextEditor");
        if (ImGui::Button("Tokenize"))
        {
            //RUN LEXER!
            std::cout << "Running Lexer\n";
        }
        ImGui::End();


        fileDialog.Display();
        if (fileDialog.HasSelected())
        {
            std::ifstream infile;
            fileName = fileDialog.GetSelected().string();
            infile.open(fileDialog.GetSelected());
            // pop up for failure? if (!infile) { //stuff }
            std::vector<std::string> lines;
            std::string line;
            while (std::getline(infile, line)) lines.push_back(line);
            editor.SetTextLines(lines);
            fileDialog.ClearSelected();
        }
        fileDialog1.Display();
        if (fileDialog.HasSelected())
        {
            std::cout << fileDialog1.GetSelected() << '\n';
            fileDialog1.ClearSelected();
        }

        /*ImGui::ShowDemoWindow();*/

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }
    ImGui::SFML::Shutdown();
}