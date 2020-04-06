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

	void TokenizeSource(std::ifstream& infile);
	void AddToken(const std::string& tok);

	bool IsDiscardableCharacter(const std::string& delimiter) const noexcept;
	bool IsCompoundOperator(const std::string& delimiter, const std::string& next) const;
	bool IsComment(const std::string& delimiter, const std::string& next) const;
	bool IsDigit(const char c) const noexcept;
	bool IsInteger(const std::string& num) const;
	bool IsCharacter(const char c) const noexcept;
	bool IsIdentifier(const std::string& identifier, const bool firstCall) const;
	//bool IsKeyword(const std::string& keyword) const;   // Not needed anymore/for now with the updated token processing
public:
	Lexer(const char* sourcePath);

	void PrintTokens() const;
	bool Done() const;
	const std::string GetLine() const;
	void Consume(const Token token);
	const TokenPair& GetCurrentToken();
};