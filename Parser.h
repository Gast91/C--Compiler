#pragma once

#include "Lexer.h"
#include "AbstractSyntaxTree.h"

class Parser
{
private:
    Lexer lexer;
    UnqPtr<ASTNode> root;

    UnqPtr<ASTNode> ParseFactor();
    UnqPtr<ASTNode> ParseTerm();
    UnqPtr<ASTNode> ParseExpr();
    UnqPtr<ASTNode> ParseBoolExpr();
    UnqPtr<ASTNode> ParseCond();
    UnqPtr<ASTNode> ParseIfCond();
    UnqPtr<ASTNode> ParseIfStatement();
    UnqPtr<ASTNode> ParseWhile();
    UnqPtr<ASTNode> ParseDoWhile();
    UnqPtr<ASTNode> ParseProgram();
    UnqPtr<ASTNode> ParseStatementBlock();
    UnqPtr<ASTNode> ParseCompoundStatement();
    std::vector<UnqPtr<ASTNode>> ParseStatementList();
    UnqPtr<ASTNode> ParseDeclarationStatement();
    UnqPtr<ASTNode> ParseStatement();
    UnqPtr<ASTNode> ParseAssignStatement();
    UnqPtr<ASTNode> ParseReturn();
    UnqPtr<ASTNode> ParseEmpty();

    bool failState = false;
    bool parsingCond = false;
public:
    Parser(const Lexer& lex);

    ASTNode* GetAST() const noexcept { return failState ? nullptr : root.get(); }
    bool Success()    const noexcept { return !failState; }
};