#pragma once
#include "Lexer.h"
#include "AbstractSyntaxTree.h"

class Parser
{
private:
    Lexer lexer;
    ASTNode* root;

    ASTNode* ParseFactor();
    ASTNode* ParseTerm();
    ASTNode* ParseExpr();
    ASTNode* ParseBoolExpr();
    ASTNode* ParseCond();
    ASTNode* ParseIf();
    ASTNode* ParseWhile();
    ASTNode* ParseDoWhile();
    ASTNode* ParseProgram();                      // main and functions and assignments/declarations - just main now - entry point
    ASTNode* ParseStatementBlock();
    ASTNode* ParseCompoundStatement();
    std::vector<ASTNode*> ParseStatementList();
    ASTNode* ParseDeclarationStatement();
    ASTNode* ParseStatement();
    ASTNode* ParseAssignStatement();
    ASTNode* ParseReturn();
    ASTNode* ParseEmpty();

    bool failState = false;
    bool parsingCond = false;
public:
    Parser(const Lexer& lex);
    ~Parser();

    ASTNode* GetAST() const noexcept { return failState ? nullptr : root; }
    bool Success()    const noexcept { return !failState; }
};