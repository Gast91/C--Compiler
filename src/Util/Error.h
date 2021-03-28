#pragma once
#include <sstream>
#include "../Lexer/Token.h"

std::string BuildSourceErrorInfo(const Token& tok, const std::string& srcCode);

class UnexpectedTokenException : public std::exception
{
private:
    static std::string errorMsg;
public:
    UnexpectedTokenException(const Token& tok, const std::string&& errorSrc) 
        : exception((errorMsg + '\'' + tok.str + "\' at " + BuildSourceErrorInfo(tok, errorSrc)).c_str()) {}
};

class SymbolNotFoundException : public std::exception
{
private:
    static std::string errorMsg;
public:
    SymbolNotFoundException(const Token& tok, const std::string&& errorSrc)
        : exception((errorMsg + '\'' + tok.str + "\' at " + BuildSourceErrorInfo(tok, errorSrc)).c_str()) {}
};

class SymbolRedefinitionException : public std::exception
{
private:
    static std::string errorMsg;
public:
    SymbolRedefinitionException(const Token& tok, const std::string&& errorSrc)
        : exception((errorMsg + '\'' + tok.str + "\' at " + BuildSourceErrorInfo(tok, errorSrc)).c_str()) {}
};