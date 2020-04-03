#include "Lexer.h"

void Lexer::TokenizeSource(std::ifstream& infile)
{
	std::string line;

	while (std::getline(infile, line))
	{
		unsigned int prev = 0, pos;
		while ((pos = line.find_first_of(" \t+-*/()=!><&|;{}", prev)) != std::string::npos)
		{
			if (pos < prev) continue;
			if (pos > prev) sourceTokens.push_back(line.substr(prev, pos - prev));
			// This delimiter is a special character or an operator (normal or compound) and must be preserved as a token 
			//or it is a form of a whitespace character and must be discarded

			// Get the delimiter that was found as a string
			std::string delimiter = line.substr(pos, 1);     // make it a char here
			// Discard whitespaces, tabs, newlines etc
			if (!IsDiscardableCharacter(delimiter))
			{
				// If the next character and the current delimiter form a compound operator,
				// they must be preserved as one token
				std::string nextCharacter = line.substr(pos + 1, 1);
				if (IsCompoundOperator(delimiter, nextCharacter))
				{
					sourceTokens.push_back(delimiter + nextCharacter);
					// Skip the next character since it has been processed already (part of a compound operator)
					prev = pos + 2;
					continue;
				}
				// Comment start at any point in this line means we skip the rest of the line
				else if (IsComment(delimiter, nextCharacter)) { pos = prev = std::string::npos; break; }
				// Delimiter is just a normal operator, preserve it
				else sourceTokens.push_back(delimiter);
			}
			prev = pos + 1;
		}
		if (prev < line.length()) sourceTokens.push_back(line.substr(prev, std::string::npos));
	}
	sourceTokens.push_back("END"); // temp end of file token
}

bool Lexer::IsDiscardableCharacter(const std::string& delimiter) { return delimiter.find_first_of(" \t\r\n") != std::string::npos; }
bool Lexer::IsCompoundOperator(const std::string& delimiter, const std::string& next)
{
	switch (delimiter[0])
	{
	case '>': case '<': case '*': case '/': case '!': case '=': return next == "=";
	case '+':  return next == "=" || next == "+";
	case '-':  return next == "=" || next == "-";
	case '&':  return next == "&";
	case '|':  return next == "|";
	default: return false;
	}
}
bool Lexer::IsComment(const std::string& delimiter, const std::string& next)  // just const char* both - what is the point of strings, same as compoundoperators
{
	return delimiter[0] == '/' && next == "/";
}
bool Lexer::IsDigit(char c)
{
	return c == '0' || c == '1' ||
		   c == '2' || c == '3' ||
		   c == '4' || c == '5' ||
		   c == '6' || c == '7' ||
		   c == '8' || c == '9';
}
bool Lexer::IsInteger(const std::string& num)
{
	unsigned int i = 0;
	char digit = num.front();
	if (IsDigit(digit))
	{
		if (++i >= num.length()) return true;
		else return IsInteger(num.substr(i, std::string::npos));
	}
	return false;
}
bool Lexer::IsCharacter(char c)
{
	return c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' || c == 'f' || c == 'g' ||
		   c == 'h' || c == 'i' || c == 'j' || c == 'k' || c == 'l' || c == 'm' || c == 'n' ||
		   c == 'o' || c == 'p' || c == 'q' || c == 'r' || c == 's' || c == 't' || c == 'u' ||
		   c == 'v' || c == 'w' || c == 'x' || c == 'y' || c == 'z' || c == 'A' || c == 'B' ||
		   c == 'C' || c == 'D' || c == 'E' || c == 'F' || c == 'G' || c == 'H' || c == 'I' ||
		   c == 'J' || c == 'K' || c == 'L' || c == 'M' || c == 'N' || c == 'O' || c == 'P' ||
		   c == 'Q' || c == 'R' || c == 'S' || c == 'T' || c == 'U' || c == 'V' || c == 'W' ||
		   c == 'X' || c == 'Y' || c == 'z' || c == '_' || IsDigit(c);
	return false;
}
bool Lexer::IsIdentifier(const std::string& identifier, const bool firstCall)
{
	if (firstCall) if (IsDigit(identifier[0])) return false;
	unsigned int i = 0;
	char character = identifier.front();
	if (IsCharacter(character))
	{
		if (++i >= identifier.length()) return true;
		else return IsIdentifier(identifier.substr(i, std::string::npos), false);
	}
	return false;
}
bool Lexer::IsKeyword(const std::string& keyword)
{
	return keyword == "int" || keyword == "if" || keyword == "else" ||
		   keyword == "while" || keyword == "for" || keyword == "return";
}

Lexer::Lexer(char* sourcePath)
{
	std::ifstream infile;

	infile.open("source.txt");  // this for debug
	//infile.open(sourcePath);

	if (!infile)
	{
		std::cout << "Cannot open file\n";
		return;
	}

	TokenizeSource(infile);
	infile.close();
}
Lexer::~Lexer() {}

void Lexer::PrintTokenizedInput()
{
	std::cout << "Tokenized Input (Split by whitespace):\n";
	for (std::string token : sourceTokens) std::cout << token << " ";
	std::cout << "\n";
}

bool Lexer::Done() { return sourceTokens.at(currentTokenIndex) == "END"; } // HACK

void Lexer::Consume(std::string token)  // make it token? NO need to return or use advance anymore, can be void
{
	if (token == sourceTokens.at(currentTokenIndex))
	{
		if (currentTokenIndex + 1 < sourceTokens.size()) { ++currentTokenIndex; }
	}
	else throw UnexpectedTokenException("Encountered unexpected token ' " + token + "', Expected: " + sourceTokens.at(currentTokenIndex)); // needs line number here preferably also handle it somewhere
}
std::string& Lexer::GetCurrentToken() { return sourceTokens.at(currentTokenIndex); }