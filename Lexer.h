#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "Error.h"
#include "Token.h"

class Lexer
{
private:
    std::vector<TokenPair> sourceTokens;
    unsigned int currentTokenIndex = 0;
    unsigned int line = 0;

    bool failState = false;

    void TokenizeSource(std::ifstream& infile);
    void AddToken(const std::string& tok);

    bool IsDiscardableCharacter(const std::string& delimiter) const noexcept;
    bool IsCompoundOperator(const std::string& delimiter, const std::string& next) const;
    bool IsComment(const std::string& delimiter, const std::string& next) const;
    bool IsInteger(const std::string& num) const;
    bool IsCharacter(const unsigned char c) const noexcept;
    constexpr bool IsIdentifier(const std::string& identifier, const bool firstCall) const;
public:
    Lexer(const char* sourcePath);

    bool Failure() const;
    void PrintTokens() const;
    bool Done() const;
    const std::string GetLine() const;
    void Consume(const Token token);
    const TokenPair& GetCurrentToken();
};