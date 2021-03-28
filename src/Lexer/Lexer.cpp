#include <TextEditor.h>
#include "Lexer.h"
#include "../Util/Logger.h"

void Lexer::Update()
{
    if (!shouldRun) return;

    input = editor->GetText();
    Reset();

    while (sourceIndex < input.size())  // Missing ':', '::', '::*' (??), '>*' (??) and potentially others
    {
        const char current = Peek();
        if (std::isspace(current))
        {
            while (std::isspace(Peek())) Advance();
            continue;
        }
        switch (current)
        {
        case '[': case ']': case '{': case '}': case '(':
        case ')': case ';': case ',': case '?': case '~': 
            AddToken({ { current }, pos });
            Advance();
            continue;
        case '>': case '<':
            StartToken();
            Advance();
            if (Peek() == current)
            {
                Advance();
                // '<<=' OR '>>="
                if (Peek() == '=')
                {
                    SubmitToken();
                    continue;
                }
                // '<<' OR '>>'
                AddToken({ input.substr(tokenStartIndex, sourceIndex - tokenStartIndex), tokenStartPos });
                continue;
            }
            if (Peek() == '=')
            {
                // '<=' OR '>='
                SubmitToken();
                continue;
            }
            // '>' OR '<'
            AddToken({ { current }, tokenStartPos });
            continue;
        case '+': case '-': case '&': case '|': case '=':
            StartToken();
            Advance();
            if (Peek() == current)
            {
                // '++' OR '--' OR '&&' or '||' or '=='
                SubmitToken();
                continue;
            }
            if (Peek() == '=')
            {
                // '+=' OR '-=' OR '&=' OR '|=' and technically '==' but it will be caught above
                SubmitToken();
                continue;
            }
            // '+' OR '-' OR '&' OR '|' OR '='
            AddToken({ { current }, tokenStartPos });
            continue;
        case '*': case '!': case '^': case '%':
            StartToken();
            Advance();
            if (Peek() == '=')
            {
                // '*=' OR '!=' OR '^=' OR '%='
                SubmitToken();
                continue;
            }
            // '*' OR '!' OR '^' OR '%'
            AddToken({ { current }, tokenStartPos });
            continue;
        case '/':                 // Comments or Division or DivisionAssignment
            if (Peek(1) == '/')
            {
                while (Peek() && Peek() != '\n') Advance();
                continue; // Single Line Comment - Ignore It
            }
            if (Peek(1) == '*')
            {
                // Multi Line Comment Start - Proccess it
                Advance(2); // '/' and '*'
                bool multilineEnd = false;
                while (Peek())
                {
                    if (Peek() == '*' && Peek(1) == '/')
                    {
                        multilineEnd = true;
                        break;
                    }
                    Advance();
                }
                if (multilineEnd) Advance(2); // '*' and '/'
                continue; // Multiline Comment End - Ignore it
            }
            if (Peek(1) == '=')
            {
                // '/='
                StartToken();
                Advance();  // '/'
                SubmitToken();
                continue;
            }
            // '/'
            AddToken({ { current }, pos });
            Advance();
            continue;
        default: if (IsQuote(TokenID::S_QUOTE) || IsQuote(TokenID::D_QUOTE) || IsRawString() || IsIdentifier(current) || IsNumber(current)) continue;
        }   

        // No clue what it is, it will be added as unknown
        Logger::Warn("Lexer encountered unknown token '{}'\n", current);
        Advance();
        AddToken({ { current }, pos });
    }

    shouldRun = false;
    if (!sourceTokens.empty())
    {
        // The last token is always the end of file, to guard against accessing past the end of sourceTokens
        sourceTokens.push_back({ "EOF", { sourceTokens.back().coords.line, 1 }, TokenID::ENDF });
        Logger::Info("Tokenized input!\n");
        for (const auto& tok : sourceTokens) Logger::Debug("{} ", tok.str);
        Logger::Debug('\n');
    }
}

void Lexer::StartToken()
{
    tokenStartIndex = sourceIndex;
    tokenStartPos = pos;
}

void Lexer::SubmitToken()
{
    Advance();
    AddToken({ input.substr(tokenStartIndex, sourceIndex - tokenStartIndex), tokenStartPos });
}

char Lexer::Advance(const size_t amount)
{
    for (int i = 0; i < amount; ++i)
    {
        const char c = input[sourceIndex++];
        if (c == '\n')
        {
            pos.line++;
            pos.col = 1;
        }
        else if (c == '\t') pos.col += editor->GetTabSize();
        else pos.col++;
    }
    return input[sourceIndex - 1];
}

void Lexer::AddToken(const Token& token)
{
    const auto [it, success] = tokens.emplace(token.str, TokenID::UNKNOWN);
    if (success && token.type != TokenID::UNKNOWN) it->second = token.type;
    sourceTokens.push_back({ token.str, token.coords, it->second });
}

void Lexer::Reset()
{
    pos = { 1, 1 };
    sourceIndex = 0;
    if (!sourceTokens.empty())
    {
        sourceTokens.clear();
        currentTokenIndex = 0;
    }
}

void Lexer::Consume(const TokenID tokenType)
{
    const auto& [tok, coords, type] = sourceTokens.at(currentTokenIndex);
    if (tokenType == type && currentTokenIndex < sourceTokens.size()) ++currentTokenIndex;
    else throw UnexpectedTokenException(GetCurrentToken(), GetSourceLine ? GetSourceLine(coords.line) : "");
}

bool Lexer::IsQuote(const TokenID quoteType)
{
    // TODO: Complain if not quotes? - Switch to templated func
    const char c = quoteType == TokenID::S_QUOTE ? '\'' : '"';

    auto matchEscapeSeq = [&]() -> size_t {
        switch (Peek(1))
        {
        case '\'': case '"': case '?': case '\\': case 'a':
        case 'b':  case 'f': case 'n': case 'r':  case 't': case 'v':
            return 2;
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
        {
            size_t octalDigits = 1;
            for (size_t i = 0; i < 2; ++i)
            {
                const char next = Peek(2 + i);
                if (next < '0' || next > '7') break;
                ++octalDigits;
            }
            return 1 + octalDigits;
        }
        case 'x':
        {
            size_t hexDigits = 0;
            while (isxdigit(Peek(2 + hexDigits))) ++hexDigits;
            return 2 + hexDigits;
        }
        case 'u': case 'U':
        {
            bool isUnicode = true;
            const size_t digitNo = Peek(1) == 'u' ? 4 : 8;
            for (size_t i = 0; i < digitNo; ++i)
            {
                if (!isxdigit(Peek(2 + i)))
                {
                    isUnicode = false;
                    break;
                }
            }
            return isUnicode ? 2 + digitNo : 0;
        }
        default: return 0;
        }
    };

    if (const size_t prefix = MatchStringPrefix(c); prefix > 0)
    {
        StartToken();
        Advance(prefix);
        while (Peek())
        {
            // Escape Sequence within quotes - FIXME: Escape sequence is split from string itself do we care?
            if (Peek() == '\\')
            {
                if (const size_t escape = matchEscapeSeq(); escape > 0)
                {
                    AddToken({ input.substr(tokenStartIndex, sourceIndex - tokenStartIndex), tokenStartPos, quoteType });
                    StartToken();
                    Advance(escape);
                    AddToken({ input.substr(tokenStartIndex, sourceIndex - tokenStartIndex), tokenStartPos, TokenID::ESCAPESEQ });
                    StartToken();
                    continue;
                }
            }
            if (quoteType == TokenID::D_QUOTE)
                if (!Peek(1)) break; // If string is not terminated - stop before EOF
            if (Advance() == c) break;
        }
        AddToken({ input.substr(tokenStartIndex, sourceIndex - tokenStartIndex), tokenStartPos, quoteType });
        return true;
    }
    return false;
}

bool Lexer::IsRawString()
{
    if (const size_t prefix = MatchStringPrefix('R'); prefix > 0 && Peek(prefix) == '"')
    {
        StartToken();
        Advance(prefix + 1);
        size_t prefixStart = sourceIndex;
        while (Peek() && Peek() != '(') Advance();
        const auto prefixString = input.substr(prefixStart, sourceIndex - prefixStart);
        while (Peek())
        {
            if (Advance() == '"')
            {
                if (input[sourceIndex - 1 - prefixString.length() - 1] == ')')
                {
                    auto suffix_string = input.substr(sourceIndex - 1 - prefixString.length(), prefixString.length());
                    if (prefixString == suffix_string) break;
                }
            }
        }
        AddToken({ input.substr(tokenStartIndex, sourceIndex - tokenStartIndex), tokenStartPos, TokenID::RAWSTR });
        return true;
    }
    return false;
}

bool Lexer::IsNumber(const char current)
{
    if (isdigit(current) || (current == '.' && isdigit(Peek(1))))
    {
        StartToken();
        Advance();

        TokenID tokType = current == '.' ? TokenID::FLOAT_LITERAL : TokenID::INT_LITERAL;
        bool isHex = false;
        bool isBinary = false;

        auto matchExponent = [&]() -> size_t {
            char ch = Peek();
            if (ch != 'e' && ch != 'E' && ch != 'p' && ch != 'P') return 0;

            tokType = TokenID::FLOAT_LITERAL;
            size_t length = 1;
            ch = Peek(length);
            if (ch == '+' || ch == '-') ++length;
            for (ch = Peek(length); isdigit(ch); ch = Peek(length)) ++length;
            return length;
        };

        auto matchTypeLiteral = [&]() {
            for (size_t length = 0;;)
            {
                const char ch = Peek(length);
                if ((ch == 'u' || ch == 'U') && tokType == TokenID::INT_LITERAL)
                    ++length;
                else if ((ch == 'f' || ch == 'F') && !isBinary)
                {
                    tokType = TokenID::FLOAT_LITERAL;
                    ++length;
                }
                else if (ch == 'l' || ch == 'L') ++length;
                else return length;
            }
        };

        if (Peek() == 'b' || Peek() == 'B')
        {
            Advance();
            isBinary = true;
            for (char ch = Peek(); ch == '0' || ch == '1' || (ch == '\'' && Peek(1) != '\''); ch = Peek()) Advance();
        }
        else
        {
            if (Peek() == 'x' || Peek() == 'X')
            {
                Advance();
                isHex = true;
            }

            for (char ch = Peek(); (isHex ? isxdigit(ch) : isdigit(ch)) || (ch == '\'' && Peek(1) != '\'') || ch == '.'; ch = Peek())
            {
                if (ch == '.')
                {
                    if (tokType == TokenID::INT_LITERAL) tokType = TokenID::FLOAT_LITERAL;
                    else break;
                }
                Advance();
            }
        }

        if (!isBinary) Advance(matchExponent());

        Advance(matchTypeLiteral());

        AddToken({ input.substr(tokenStartIndex, sourceIndex - tokenStartIndex), tokenStartPos, tokType });
        return true;
    }
    return false;
}

bool Lexer::IsIdentifier(const char current)
{
    auto IsValidIdentifierStart = [](const char c) { return std::isalpha(c) || c == '_' || c == '$'; };
    auto IsValidIdentifierMid   = [&](const char c) { return std::isdigit(c) || IsValidIdentifierStart(c); };

    if (IsValidIdentifierStart(current))
    {
        StartToken();
        while (Peek() && IsValidIdentifierMid(Peek())) Advance();
        AddToken({ input.substr(tokenStartIndex, sourceIndex - tokenStartIndex), tokenStartPos, TokenID::IDENTIFIER });
        return true;
    }
    return false;
}

size_t Lexer::MatchStringPrefix(const char quote) const
{
    if (Peek() == quote) return 1;
    if (Peek() == 'L' && Peek(1) == quote) return 2;
    if (Peek() == 'u')
    {
        if (Peek(1) == quote) return 2;
        if (Peek(1) == '8' && Peek(2) == quote) return 3;
    }
    if (Peek() == 'U' && Peek(1) == quote) return 2;
    return 0;
}