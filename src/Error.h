#pragma once
#include <string>

struct ErrorInfo
{
    const std::string sourceLoc;
    const std::string sourceCode;
};

class UnexpectedTokenException : public std::exception
{
public:
    UnexpectedTokenException(const std::string& msg) : exception(("\nERROR: " + msg).c_str()) {}  // PARSER ERROR
    UnexpectedTokenException(const ErrorInfo e, const std::string& msg) : exception(("\n" + e.sourceLoc + " PARSER ERROR: " + msg + "\n" + e.sourceCode).c_str()) {}
};

class SymbolNotFoundException : public std::exception
{
public:
    SymbolNotFoundException(const std::string& msg) : exception(("\nERROR: " + msg).c_str()) {}  // SEMANTIC ANALYZER 
};

class SymbolRedefinitionException : public std::exception
{
public:
    SymbolRedefinitionException(const std::string& msg) : exception(("\nERROR: " + msg).c_str()) {} // SEMANTIC ANALYZER
};