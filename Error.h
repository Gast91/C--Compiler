#pragma once
#include <string>

class UnexpectedTokenException : public std::exception
{
public:
	UnexpectedTokenException(std::string msg) : exception(("PARSER ERROR: " + msg).c_str()) {}
};

class SymbolNotFoundException : public std::exception
{
public:
	SymbolNotFoundException(std::string msg) : exception(("SEMANTIC ANALYZER ERROR: " + msg).c_str()) {}
};

class SymbolRedefinitionException : public std::exception
{
public:
	SymbolRedefinitionException(std::string msg) : exception(("SEMANTIC ANALYZER ERROR: " + msg).c_str()) {}
};