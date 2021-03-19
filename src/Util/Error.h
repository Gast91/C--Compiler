#pragma once
#include <sstream>
#include "../Lexer/Token.h"

std::string BuildSourceErrorInfo(const TokenInfo tok, const std::string& srcCode);

class UnexpectedTokenException : public std::exception
{
private:
    static std::string errorMsg;
public:
    UnexpectedTokenException(const TokenInfo tokInfo, const std::string&& errorSrc) 
        : exception((errorMsg + '\'' + std::get<0>(tokInfo) + "\' at " + BuildSourceErrorInfo(tokInfo, errorSrc)).c_str()) {}
};

class SymbolNotFoundException : public std::exception
{
private:
    static std::string errorMsg;
public:
    SymbolNotFoundException(const TokenInfo tokInfo, const std::string&& errorSrc)
        : exception((errorMsg + '\'' + std::get<0>(tokInfo) + "\' at " + BuildSourceErrorInfo(tokInfo, errorSrc)).c_str()) {}
};

class SymbolRedefinitionException : public std::exception
{
private:
    static std::string errorMsg;
public:
    SymbolRedefinitionException(const TokenInfo tokInfo, const std::string&& errorSrc)
        : exception((errorMsg + '\'' + std::get<0>(tokInfo) + "\' at " + BuildSourceErrorInfo(tokInfo, errorSrc)).c_str()) {}
};