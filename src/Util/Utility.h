#pragma once

class ASTNode;

namespace Util // General Helpers
{
    std::string GenerateID(const ASTNode* node, const char* ID);
}

namespace GUI  // GUI Helpers
{
    struct ActionButton
    {
        const char* label;
        float width;
        std::function<void()> action;
    };

    void HelpMarker(const char* desc);

    void ShowJsonButton(ASTNode* AST);
}

class TextEditor;
namespace FD // FileDialog Helpers
{
    static const char* saveFileFilter = ".h,.hpp,.cpp,.txt";
    static const char* openFileFilter = "Source files (*.cpp *.h *.hpp *.txt){.cpp,.h,.hpp,.txt}";
    static const char* tip = "Don't forget to select the type you want to save your file as!";
#ifdef _DEBUG // USER SPECIFIC
    static const char* dialogDir = "D:/Desktop/CTests";
#else
    static const char* dialogDir = ".";
#endif

    [[maybe_unused]] bool PrintToFile(const TextEditor* editor, const std::string& filePath);

    void Save(const TextEditor* editor, const std::string& filePath);
}