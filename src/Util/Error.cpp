#include "Error.h"

std::string BuildSourceErrorInfo(const Token& tok, const std::string& srcCode)
{
    static const std::string sourceTag = "<source> ";
    static const std::string seperator = "\t|\t\t";

    const auto& [name, coords, type] = tok;
    const std::string errorLoc { sourceTag + std::to_string(coords.line) + ':' + std::to_string(coords.col) + ' ' };
    std::string errorMarker;
    std::stringstream ss;

    if (srcCode != "")
        errorMarker.append(coords.col - 1, ' ').append("^").append(tok.str.size() - 1, '~');
    ss << errorLoc  << '\n' << seperator << srcCode << '\n' << seperator << errorMarker;

    return ss.str();
}

std::string UnexpectedTokenException::errorMsg = "[PARSER ERROR]: Encountered unexpected Token ";
std::string SymbolNotFoundException::errorMsg = "[SEMANTIC ERROR]: Redefinition of identifier ";
std::string SymbolRedefinitionException::errorMsg = "[SEMANTIC ERROR]: Redefinition of identifier ";