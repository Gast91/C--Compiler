#pragma once
#include <string>
#include <vector>

#include "Token.h"
#include "../Util/Error.h"
#include "../Util/ModuleManager.h"

class TextEditor;
class Lexer : public IObserver<>
{
private:
    TextEditor* editor;

    std::vector<Token> sourceTokens;
    size_t currentTokenIndex = 0;
    size_t sourceIndex = 0;
    size_t tokenStartIndex = 0;

    TokenCoords pos;
    TokenCoords tokenStartPos;

    std::string input;

    bool shouldRun = false;

    bool IsQuote(const TokenID quoteType);
    bool IsRawString();
    bool IsNumber(const char current);
    bool IsIdentifier(const char current);

    char Peek(const size_t amount = 0) const { return sourceIndex + amount >= input.size() ? 0 : input.at(sourceIndex + amount); }
    char Advance(const size_t amount = 1);
    void StartToken();
    void SubmitToken();
    void AddToken(const Token& token);
    size_t MatchStringPrefix(const char quote) const;
public:
    Lexer(TextEditor* ed) : editor(ed) {}
    virtual ~Lexer() = default;

    void Consume(const TokenID tokenType);

    const Token& GetCurrentToken()        const { return sourceTokens.at(currentTokenIndex); }
    std::string GetCurrentTokenVal()      const { return sourceTokens.at(currentTokenIndex).str; }
    TokenID GetCurrentTokenType()         const { return sourceTokens.at(currentTokenIndex).type; }
    size_t GetCurrentTokenLine()          const { return sourceTokens.at(currentTokenIndex).coords.line; }
    size_t GetCurrentTokenCol()           const { return sourceTokens.at(currentTokenIndex).coords.line; }
    const std::vector<Token>& GetTokens() const { return sourceTokens; }

    bool Done()       const { return sourceTokens.empty() || GetCurrentTokenType() == TokenID::ENDF; }
    bool HasTokens()  const { return !sourceTokens.empty(); }
    void ResetIndex()       { currentTokenIndex = 0; }

    // Inherited via IObserver
    virtual bool ShouldRun() const override { return shouldRun; }
    virtual void SetToRun()        override { shouldRun = true; }
    virtual void Update()          override;
    virtual void Reset()           override;
};