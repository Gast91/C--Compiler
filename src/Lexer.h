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
    //std::vector<TokenPair> sourceTokens;
    std::vector<TokenInfo> sourceTokens;
    unsigned int currentTokenIndex = 0;
    //unsigned int line = 0;
    struct SourceInfo
    {
        unsigned int line = 0;
        unsigned int column = 1;
        unsigned int lineStartIndex = 0;
    } sourceInfo;

    bool failState = false;

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
    void PrintTokens() const;
    /*bool Done() const;*/
    /*const ErrorInfo GetErrorInfo() const;*/
    const std::string GetLine() const;
    /*void Consume(const Token token);
    const TokenPair& GetCurrentToken();*/
};