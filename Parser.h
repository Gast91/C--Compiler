#pragma once

#include "Lexer.h"
#include "AbstractSyntaxTree.h"

class Parser
{
private:
    Lexer lexer;
    ASTNodePtr root;

    ASTNodePtr ParseFactor();
    ASTNodePtr ParseTerm();
    ASTNodePtr ParseExpr();
    ASTNodePtr ParseBoolExpr();
    ASTNodePtr ParseCond();
    ASTNodePtr ParseIfCond();
    ASTNodePtr ParseIfStatement();
    ASTNodePtr ParseWhile();
    ASTNodePtr ParseDoWhile();
    ASTNodePtr ParseProgram();
    ASTNodePtr ParseStatementBlock();
    ASTNodePtr ParseCompoundStatement();
    std::vector<ASTNodePtr> ParseStatementList();
    ASTNodePtr ParseDeclarationStatement();
    ASTNodePtr ParseStatement();
    ASTNodePtr ParseAssignStatement();
    ASTNodePtr ParseReturn();
    ASTNodePtr ParseEmpty();

    bool failState = false;
    bool parsingCond = false;
public:
    Parser(const Lexer& lex);

    ASTNode* GetAST() const noexcept { return failState ? nullptr : root.get(); }  // ?????
    bool Success()    const noexcept { return !failState; }
};