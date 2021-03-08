#include <imgui.h>
#include <imgui-SFML.h>

#include <TextEditor.h>
#include <ImGuiFileDialog.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <magic_enum.hpp>

#include <fstream>

#include "Lexer.h"
#include "Parser.h"

constexpr const char* fileTypeFilter = "Source files (*.cpp *.h *.hpp *.txt){.cpp,.h,.hpp,.txt}";

static void HelpMarker(const char* desc)
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
// Usage:
//  static ExampleAppLog my_log;
//  my_log.AddLog("Hello %d world\n", 123);
//  my_log.Draw("title");
struct ExampleAppLog
{
    ImGuiTextBuffer     Buf;
    ImGuiTextFilter     Filter;
    ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
    bool                AutoScroll;  // Keep scrolling if already at the bottom.

    ExampleAppLog()
    {
        AutoScroll = true;
        Clear();
    }

    void Clear()
    {
        Buf.clear();
        LineOffsets.clear();
        LineOffsets.push_back(0);
    }

    void AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        int old_size = Buf.size();
        va_list args;
        va_start(args, fmt);
        Buf.appendfv(fmt, args);
        va_end(args);
        for (int new_size = Buf.size(); old_size < new_size; old_size++)
        {
            if (Buf[old_size] == '\n')  
                LineOffsets.push_back(old_size + 1);
        }
    }

    void Draw(const char* title, bool* p_open = NULL)
    {
        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }

        // Options menu
        if (ImGui::BeginPopup("Options"))
        {
            ImGui::Checkbox("Auto-scroll", &AutoScroll);
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
        Filter.Draw("Filter", -100.0f);

        ImGui::Separator();
        ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        if (clear) Clear();
        if (copy)  ImGui::LogToClipboard();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        const char* buf = Buf.begin();
        const char* buf_end = Buf.end();
        if (Filter.IsActive())
        {
            // In this example we don't use the clipper when Filter is enabled.
            // This is because we don't have a random access on the result on our filter.
            // A real application processing logs with ten of thousands of entries may want to store the result of
            // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
            for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
            {
                const char* line_start = buf + LineOffsets[line_no];
                const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                if (Filter.PassFilter(line_start, line_end))
                    ImGui::TextUnformatted(line_start, line_end);   // something else? for format and wrap
            }
        }
        else
        {
            // The simplest and easy way to display the entire buffer:
            //   ImGui::TextUnformatted(buf_begin, buf_end);
            // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
            // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
            // within the visible area.
            // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
            // on your side is recommended. Using ImGuiListClipper requires
            // - A) random access into your data
            // - B) items all being the  same height,
            // both of which we can handle since we an array pointing to the beginning of each line of text.
            // When using the filter (in the block of code above) we don't have random access into the data to display
            // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
            // it possible (and would be recommended if you want to search through tens of thousands of entries).
            ImGuiListClipper clipper;
            clipper.Begin(LineOffsets.Size);
            while (clipper.Step())
            {
                for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                {
                    const char* line_start = buf + LineOffsets[line_no];
                    const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                    ImGui::TextUnformatted(line_start, line_end);    // something else? for format and wrap
                }
            }
            clipper.End();
        }
        ImGui::PopStyleVar();

        if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
        ImGui::End();
    }
};

// Demonstrate creating a simple log window with basic filtering.
static void ShowExampleAppLog(bool* p_open, ExampleAppLog& log)
{
    //static ExampleAppLog log;

    // For the demo: add a debug button _BEFORE_ the normal log window contents
    // We take advantage of a rarely used feature: multiple calls to Begin()/End() are appending to the _same_ window.
    // Most of the contents of the window will be added by the log.Draw() call.
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Console Output", p_open);
    if (ImGui::SmallButton("[Debug] Add 5 entries"))
    {
        static int counter = 0;
        const char* categories[3] = { "info", "warn", "error" };
        const char* words[] = { "Bumfuzzled", "Cattywampus", "Snickersnee", "Abibliophobia", "Absquatulate", "Nincompoop", "Pauciloquent" };
        for (int n = 0; n < 5; n++)
        {
            const char* category = categories[counter % IM_ARRAYSIZE(categories)];
            const char* word = words[counter % IM_ARRAYSIZE(words)];
            log.AddLog("[%05d] [%s] Hello, current time is %.1f, here's a word: '%s'\n",
                ImGui::GetFrameCount(), category, ImGui::GetTime(), word);
            counter++;
        }
    }
    ImGui::End();

    // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
    log.Draw("Console Output", p_open);
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

    Lexer lexer;
    Parser parser;
    parser.RegisterLexer(&lexer);  // ONE CONSTRUCTOR - NOT LIKE THIS
    ExampleAppLog log;
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
                break;
            default: break;
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        auto cpos = editor.GetCursorPosition();
        ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar /*| ImGuiWindowFlags_NoMove*/);  // change for docking
        ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New"))            { editor.SetText(""); fileName = "Untitled"; }  //?? handle file name and extension seperately?
                if (ImGui::MenuItem("Open", "Ctrl+O")) { ImGuiFileDialog::Instance()->OpenModal("ChooseFileKey", "Choose File", fileTypeFilter, ".", ""); }
                if (ImGui::MenuItem("Save", "Ctrl+S"))
                {
                    if (fileName == "Untitled") ImGuiFileDialog::Instance()->OpenModal("SaveAsKey", "Save File As", fileTypeFilter, ".", "");
                    else
                    {
                        std::ofstream outfile;
                        outfile.open(fileName);
                        outfile << std::noskipws;
                        // pop up for failure? if (!outfile) { //stuff }
                        outfile << editor.GetText();
                        outfile.close();
                    }
                }
                if (ImGui::MenuItem("Save As..")) { ImGuiFileDialog::Instance()->OpenModal("SaveAsKey", "Save File As", fileTypeFilter, ".", ""); }
                
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
        if (ImGui::SmallButton("Parse"))  // Generate AST?
        {
            // if there was a change in the editor
            // run all previous steps and update all
            // windows up to AST
            try 
            { 
                parser.Parse();
                if (parser.Success()) log.AddLog("%s", "Parsing successful, AST built\n"); // kinda meh    
            }
            catch (UnexpectedTokenException& ex) { log.AddLog("%s", ex.what()); }
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
            //lexer.PrintTokens();                    // PrintTokens rendering ImGui
            for (const auto& toks : lexer.GetTokens()) log.AddLog("%s ", std::get<0>(toks).c_str());
            log.AddLog("%c", '\n');
        }
        TokButtonWidth = ImGui::GetItemRectSize().x;       

        ImGui::Separator();
        ImGui::Spacing();
        
        editor.Render("TextEditor");  
        ImGui::End();


        // MAKE THIS PART OF THE LEXER.PRINT????
        static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable; // | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;
        ImGui::Begin("Lexer Output");
        if (ImGui::BeginTable("Tokens", 3, flags))
        {
            // Submit columns name with TableSetupColumn() and call TableHeadersRow() to create a row with a header in each column.
            // (Later we will show how TableSetupColumn() has other uses, optional flags, sizing weight etc.)
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

        /*
        *   IMGUI TREE SHOWING NODE HIERARCHY - BECOMES THE NEW ASTVISUALIZER?? (HANDLES .JS FILE OUTPUT AND IMGUI TREE (CONSOLE?)
        *   EACH NODE DOES ITS OWN IMGUI APPEND?
        */
        

        if (ImGuiFileDialog::Instance()->Display("ChooseFileKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::ifstream infile;
                fileName = ImGuiFileDialog::Instance()->GetFilePathName(); // GetSelectionDifferences?
                infile.open(fileName);
                // pop up for failure? if (!infile) { //stuff }
                std::vector<std::string> lines;
                std::string line;
                while (std::getline(infile, line)) lines.push_back(line);
                editor.SetTextLines(lines);

                infile.close();
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (ImGuiFileDialog::Instance()->Display("SaveAsKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::ofstream outfile;
                std::string fpName = ImGuiFileDialog::Instance()->GetFilePathName();
                outfile.open(fpName);
                outfile << std::noskipws;
                // pop up for failure? if (!outfile) { //stuff }
                outfile << editor.GetText();
                fileName = fpName;
                outfile.close();

            }
            ImGuiFileDialog::Instance()->Close();
        }
        static bool* p_open;
        ShowExampleAppLog(p_open, log);
        //ImGui::ShowDemoWindow();
        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }
    ImGui::SFML::Shutdown();
}

/*  TODO:
*       Overwriting confirmation for SaveAs
*       Popup dialog for file errors
*       Shortcut implementation
*       Cleanup/Separation of display etc
*       Docking - SFML backend issues
*       Merge all compiler branches, create new on (dissertation)
*           Master becomes an ImGui frontend
*/