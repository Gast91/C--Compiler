#include "Lexer.h"

Lexer::Lexer(const char* sourcePath)
{
    std::ifstream infile;

    infile.open(sourcePath);

    if (!infile)
    {
        std::cout << "Cannot open file\n";
        return;
    }

    TokenizeSource(infile);
    infile.close();
}

void Lexer::TokenizeSource(std::ifstream& infile)
{
    std::string srcLine;
    while (std::getline(infile, srcLine))
    {
        // Add a new line character to the token vector to let the lexer know when a new line is reached
        // when returning the tokens. Used for token line tracking for errors.
        sourceTokens.push_back(std::make_pair("\n", Token::NLINE));

        unsigned int prev = 0, pos;
        while ((pos = srcLine.find_first_of(" \t+-*/()=!><&|;{}%", prev)) != std::string::npos)
        {
            if (pos < prev) continue;
            if (pos > prev) AddToken(srcLine.substr(prev, pos - prev));

            // This delimiter is a special character or an operator (normal or compound) and must be preserved as a token 
            // or it is a form of a whitespace character and must be discarded
            const std::string delimiter = srcLine.substr(pos, 1);
            // Discard whitespaces, tabs, newlines etc
            if (!IsDiscardableCharacter(delimiter))
            {
                // If the next character and the current delimiter form a compound operator,
                // they must be preserved as one token
                const std::string nextCharacter = srcLine.substr(pos + 1, 1);
                if (IsCompoundOperator(delimiter, nextCharacter))  // doesnt check for >>= or <<=
                {
                    AddToken(delimiter + nextCharacter);
                    // Skip the next character since it has been processed already (part of a compound operator)
                    prev = pos + 2;
                    continue;
                }
                // Comment start at any point in this line means we skip the rest of the line
                else if (IsComment(delimiter, nextCharacter)) { pos = prev = std::string::npos; break; }
                // Delimiter is just a normal operator, preserve it
                else AddToken(delimiter);
            }
            prev = pos + 1;
        }
        if (prev < srcLine.length()) AddToken(srcLine.substr(prev, std::string::npos));
    }
    AddToken("\032");  // End of file Token
    while (sourceTokens.at(currentTokenIndex).second == Token::NLINE) { ++line; ++currentTokenIndex; }
}

void Lexer::AddToken(const std::string& tok)
{
    // Get iterator to token if already in the map (this will be the case for defined language features and operators)
    // but not for literals and identifiers or things that do not fall in either of the aforementioned categories.
    const auto[it, success] = tokens.emplace(tok, Token::UNKNOWN);
    if (success)
    {
        // Succesful insertion has the identifiers, literal or garbage token marked as unknown
        // Update it if it satisfies the criteria for a literal or identifier
        if (IsIdentifier(tok, true)) it->second = Token::IDENTIFIER;
        else if (IsInteger(tok))     it->second = Token::INT_LITERAL;
        // Or it is a not yet defined language feature or garbage token so it stays unknown
    }
    sourceTokens.push_back(std::make_pair(it->first, it->second));
}

bool Lexer::IsDiscardableCharacter(const std::string& delimiter) const noexcept { return delimiter.find_first_of(" \t\r\n") != std::string::npos; }

bool Lexer::IsCompoundOperator(const std::string& delimiter, const std::string& next) const
{
    switch (delimiter.front())  // shiftleft and shiftright missing
    {
    case '*': case '/': case '!': case '=': case '%': case '^': return next == "=";
    case '+': return next == "=" || next == "+";
    case '-': return next == "=" || next == "-";
    case '&': return next == "&" || next == "=";
    case '|': return next == "|" || next == "=";
    case '>': return next == ">";
    case '<': return next == "<";
    default:  return false;
    }
}

bool Lexer::IsComment(const std::string& delimiter, const std::string& next) const
{
    return delimiter.front() == '/' && next == "/";
}

bool Lexer::IsInteger(const std::string& num) const
{
    unsigned int i = 0;
    const unsigned char digit = num.front();
    if (isdigit(digit))
    {
        if (++i >= num.length()) return true;
        else return IsInteger(num.substr(i, std::string::npos));
    }
    return false;
}

// Checks if the character pass to it is one of the alphabet characters (lower or upper) or a digit or underscore
bool Lexer::IsCharacter(const unsigned char c) const noexcept { return isalpha(c) || c == '_' || isdigit(c); }

constexpr bool Lexer::IsIdentifier(const std::string& identifier, const bool firstCall) const
{
    if (firstCall) if (isdigit(identifier.front())) return false;
    unsigned int i = 0;
    const char character = identifier.front();
    if (IsCharacter(character))
    {
        if (++i >= identifier.length()) return true;
        else return IsIdentifier(identifier.substr(i, std::string::npos), false);
    }
    return false;
}

void Lexer::PrintTokens() const
{
    std::cout << "Tokenized Input (Split by whitespace):\n";
    for (const auto& token : sourceTokens) if (token.second != Token::NLINE && token.second != Token::FILE_END) std::cout << token.first << ' ';
    std::cout << '\n';
}

bool Lexer::Done() const { return sourceTokens.at(currentTokenIndex).second == Token::FILE_END; }

const std::string Lexer::GetLine() const { return std::to_string(line); }

void Lexer::Consume(const Token token)
{
    if (token == sourceTokens.at(currentTokenIndex).second && currentTokenIndex + 1 < sourceTokens.size()) ++currentTokenIndex;
    else throw UnexpectedTokenException("Encountered unexpected token at line " + GetLine());
    while (sourceTokens.at(currentTokenIndex).second == Token::NLINE) { ++line; ++currentTokenIndex; }
}

const TokenPair& Lexer::GetCurrentToken() { return sourceTokens.at(currentTokenIndex); }