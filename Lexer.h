#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "Error.h"

class Lexer
{
private:
	friend class Parser;

	std::vector<std::string> sourceTokens;  // switch to array of Tokens (since it has already been processed it can same time)
	int currentTokenIndex = 0;
	int line = 1;

	// Tokenize source based on the specified delimiters (preserving those delimiters if not whitespace characters)
	void TokenizeSource(std::ifstream& infile);
	// Token recognizers - some can be const char* - more efficient than memory alloc
	bool IsDiscardableCharacter(const std::string& delimiter);
	bool IsCompoundOperator(const std::string& delimiter, const std::string& next);  // UNUSED
	bool IsComment(const std::string& delimiter, const std::string& next);
	bool IsDigit(char c);
	bool IsInteger(const std::string& num);
	bool IsCharacter(char c);
	bool IsIdentifier(const std::string& identifier, const bool firstCall);
	bool IsKeyword(const std::string& keyword);   // UNUSED
public:
	Lexer(char* sourcePath);  // const
	~Lexer();

	void PrintTokenizedInput();
	bool Done();
	void Consume(std::string token); // const ptr?
	std::string& GetCurrentToken();
};