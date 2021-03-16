#pragma once
#include <string>
#include <vector>
#include <TextEditor.h>

#include "Token.h"
#include "../Util/Error.h"
#include "../Util/ModuleManager.h"

class Lexer : public IObserver
{
private:
    TextEditor* editor;

    std::vector<TokenInfo> sourceTokens;
    unsigned int currentTokenIndex = 0;

    bool shouldRun = false;

    void AddToken(const std::string& tok, const size_t lineNo, const size_t col);

    bool IsDiscardableCharacter(const std::string& delimiter) const noexcept;
    bool IsCompoundOperator(const std::string& delimiter, const std::string& next) const;
    //size_t OperatorWidth(const std::string& delimiter, const std::string& twoNext) const;
    bool IsMultiLineCommentStart(const std::string& delimiter, const std::string& next) const;
    bool IsMultiLineCommentEnd(const std::string& delimiter, const std::string& next) const;
    bool ShouldSkipChars(const std::string& line, size_t& pos, size_t& prev, size_t& col, bool& multiline);
    bool IsComment(const std::string& delimiter, const std::string& next) const;
    bool IsInteger(const std::string& num) const;
    bool IsCharacter(const unsigned char c) const noexcept;
    constexpr bool IsIdentifier(const std::string& identifier, const bool firstCall) const;

    std::string GetSourceLine(const int line, const int col);
public:
    Lexer(TextEditor* ed) : editor(ed) {}
    virtual ~Lexer() = default;

    void Consume(const Token tokenType);

    size_t GetTokenNumber()                   const { return sourceTokens.size(); }
    const TokenInfo& GetCurrentToken()        const { return sourceTokens.at(currentTokenIndex); }
    std::string GetCurrentTokenVal()          const { return std::get<0>(sourceTokens.at(currentTokenIndex)); }
    Token GetCurrentTokenType()               const;
    std::string GetCurrentTokenLine()         const { return std::to_string(std::get<2>(sourceTokens.at(currentTokenIndex))); }
    std::string GetCurrentTokenCol()          const { return std::to_string(std::get<3>(sourceTokens.at(currentTokenIndex))); }
    const std::vector<TokenInfo>& GetTokens() const { return sourceTokens; }

    const ErrorInfo GetErrorInfo();

    bool Done() const { return currentTokenIndex == sourceTokens.size(); }
    void ResetIndex() { currentTokenIndex = 0; }

    // Inherited via IObserver
    virtual bool ShouldRun() const override { return shouldRun; }
    virtual void SetToRun() override { shouldRun = true; }
    virtual void Run() override;
};