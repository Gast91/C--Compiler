#pragma once
#include <string>

class UnexpectedTokenException : public std::exception
{
public:
    UnexpectedTokenException(const std::string& msg) : exception(("PARSER ERROR: " + msg).c_str()) {}
};

class SymbolNotFoundException : public std::exception
{
public:
    SymbolNotFoundException(const std::string& msg) : exception(("SEMANTIC ANALYZER ERROR: " + msg).c_str()) {}
};

class SymbolRedefinitionException : public std::exception
{
public:
    SymbolRedefinitionException(const std::string& msg) : exception(("SEMANTIC ANALYZER ERROR: " + msg).c_str()) {}
};