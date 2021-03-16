#include "Lexer.h"
#include "../Util/Logger.h"

void Lexer::Run()
{
    if (!shouldRun) return;

    const auto& srcLines = editor->GetTextLines();
    // Reset Lexer state for next run
    if (!sourceTokens.empty())
    {
        sourceTokens.clear();
        currentTokenIndex = 0;
    }
    size_t lineNo = 0;
    bool multiline = false;
    for (const auto& line : srcLines)
    {
        ++lineNo;
        size_t prev = 0, col = 1, pos;
        while ((pos = line.find_first_of(" \t+-*/()=!><&|;{}%,.\'\"", prev)) != std::string::npos) // TODO: .'" will need a different approach
        {
            // Skip characters part of multiline comments and adjust pointer positions and flags
            if (ShouldSkipChars(line, pos, prev, col, multiline)) continue;

            if (pos < prev) continue;
            if (pos > prev)
            {
                AddToken(line.substr(prev, pos - prev), lineNo, col);
                col += pos - prev;
            }

            // This delimiter is a special character or an operator (normal or compound) and must be preserved as a token 
            // or it is a form of a whitespace character and must be discarded
            const std::string delimiter = line.substr(pos, 1);
            // Discard whitespaces, tabs, newlines etc
            if (!IsDiscardableCharacter(delimiter))
            {
                // If the next character and the current delimiter form a compound operator,
                // they must be preserved as one token
                const std::string nextCharacter = line.substr(pos + 1, 1);
                if (IsCompoundOperator(delimiter, nextCharacter))  // doesnt check for >>= or <<= and other 3 wide compound operators
                {
                    AddToken(delimiter + nextCharacter, lineNo, col);
                    col += 2;
                    // Skip the next character since it has been processed already (part of a compound operator)
                    prev = pos + 2;
                    continue;
                }
                // Comment start at any point in this line means we skip the rest of the line
                else if (IsComment(delimiter, nextCharacter)) { pos = prev = std::string::npos; break; }
                else if (IsMultiLineCommentStart(delimiter, nextCharacter)) { col += 2; multiline = true; }
                // Delimiter is just a normal operator, preserve it
                else AddToken(delimiter, lineNo, col++);
            }
            else if (delimiter == "\t") col += editor->GetTabSize(); // Tab representation in text is worth more
            else if (delimiter == " ")  col += 1;
            prev = pos + 1;
        }
        if (prev < line.length()) AddToken(line.substr(prev, std::string::npos), lineNo, col);
    }
    shouldRun = false;
    if (!sourceTokens.empty())
    {
        Logger::Info("Tokenized input!\n");
        for (const auto& tok : sourceTokens) Logger::Debug("%s ", std::get<0>(tok).c_str());
        Logger::Debug("\n");
    }
}

std::string Lexer::GetSourceLine(const int line, const int col)
{
    const auto currentCoords = editor->GetCursorPosition();
    editor->SetCursorPosition({ line, col });
    std::string sourceLine = editor->GetCurrentLineText();
    editor->SetCursorPosition(currentCoords);
    return sourceLine;
}

void Lexer::AddToken(const std::string& tok, const size_t lineNo, const size_t col)
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
    sourceTokens.push_back(std::make_tuple(it->first, it->second, lineNo, col));
}

bool Lexer::IsDiscardableCharacter(const std::string& delimiter) const noexcept { return delimiter.find_first_of(" \t\r\n") != std::string::npos; }

bool Lexer::IsCompoundOperator(const std::string& delimiter, const std::string& next) const
{
    switch (delimiter.front())  // shiftleft/shiftright assignment missing
    {
    case '*': case '/': case '!': case '=': case '%': case '^': return next == "=";
    case '+': return next == "=" || next == "+";
    case '-': return next == "=" || next == "-";
    case '&': return next == "&" || next == "=";
    case '|': return next == "|" || next == "=";
    case '>': return next == ">" || next == "=";
    case '<': return next == "<" || next == "=";
    default:  return false;
    }
}

bool Lexer::IsComment(const std::string& delimiter, const std::string& next) const
{
    return delimiter.front() == '/' && next == "/";
}

bool Lexer::ShouldSkipChars(const std::string& line, size_t& pos, size_t& prev, size_t& col, bool& multiline)  // VERY MEH..but it works
{
    if (const std::string delimiter = line.substr(pos, 1); multiline && !IsMultiLineCommentEnd(delimiter, line.substr(pos + 1, 1))) 
    {
        col += pos - prev;
        prev = pos + 1;
        if      (delimiter == "\t") col += editor->GetTabSize();
        else if (delimiter == " ")  col += 1;
        return true;
    }
    else if (IsMultiLineCommentEnd(delimiter, line.substr(pos + 1, 1)))
    {
        multiline = false;
        prev = pos + 2;
        col += 3;  // +1 for the delimiter
        return true;
    }
    return false;
}

bool Lexer::IsMultiLineCommentStart(const std::string& delimiter, const std::string& next) const
{
    return delimiter.front() == '/' && next == "*";
}
bool Lexer::IsMultiLineCommentEnd(const std::string& delimiter, const std::string& next) const
{
    return delimiter.front() == '*' && next == "/";
}

bool Lexer::IsInteger(const std::string& num) const  // needs a generalized number (floats etc) - takes into account period
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

// Checks if the character passed to it is one of the alphabet characters (lower or upper) or a digit or underscore
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

Token Lexer::GetCurrentTokenType() const 
{ 
    if (currentTokenIndex == sourceTokens.size())
        throw UnexpectedTokenException("Encountered Unexpected Token 'FILE_END'");
    return std::get<1>(sourceTokens.at(currentTokenIndex)); 
}

const ErrorInfo Lexer::GetErrorInfo()  // what about semantic errors?
{          
    const auto& [tok, type, line, col] = sourceTokens.at(currentTokenIndex);
    const std::string loc = "<source>::" + std::to_string(line) + ":" + std::to_string(col) + ":";
    const std::string seperator = "\t|\t\t";
    std::string codeLine = seperator + GetSourceLine(line - 1, 0);
    codeLine.append("\n" + seperator).append(col - 1, ' ').append("^").append(std::get<0>(sourceTokens.at(currentTokenIndex)).size() - 1, '~');
    return { loc, codeLine };
}

void Lexer::Consume(const Token tokenType)
{
    const auto& [tok, type, line, col] = sourceTokens.at(currentTokenIndex);
    if (tokenType == type && currentTokenIndex < sourceTokens.size()) ++currentTokenIndex;
    else throw UnexpectedTokenException(GetErrorInfo(), "Encountered Unexpected Token '" + std::get<0>(sourceTokens.at(currentTokenIndex)) + '\'');
}