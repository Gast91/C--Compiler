#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "Error.h"
#include "Token.h"

class Lexer
{
private:
    std::vector<TokenInfo> sourceTokens;
    unsigned int currentTokenIndex = 0;
    unsigned int lineStartTokenIndex = 0;

    bool failState = false;
    bool tokenized = false;

    void AddToken(const std::string& tok, const size_t lineNo, const size_t col);

    bool IsDiscardableCharacter(const std::string& delimiter) const noexcept;
    bool IsCompoundOperator(const std::string& delimiter, const std::string& next) const;
    bool IsComment(const std::string& delimiter, const std::string& next) const;
    bool IsInteger(const std::string& num) const;
    bool IsCharacter(const unsigned char c) const noexcept;
    constexpr bool IsIdentifier(const std::string& identifier, const bool firstCall) const;
public:
    void Tokenize(const std::vector<std::string>& srcLines);
    const std::vector<TokenInfo>& GetTokens() const;
    bool Failure() const;
    bool Done() const;
    const ErrorInfo GetErrorInfo() const;      // OBSOLETE ??
    std::string GetCurrentTokenVal() const;
    std::string GetCurrentTokenLine() const;
    Token GetCurrentTokenType() const;
    void Consume(const Token tokenType);
    const TokenInfo& GetCurrentToken();

    bool hasTokenized() const;
};