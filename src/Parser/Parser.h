#pragma once
#include "../Lexer/Lexer.h"
#include "../AST/AbstractSyntaxTree.h"

class Parser : public IObserver<>, public Subject<ASTNode>
{
private:
    Lexer* lexer;
    UnqPtr<ASTNode> root;

    bool failState = false;
    bool parsingCond = false;
    bool shouldRun = false;

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
public:
    Parser(Lexer* lex) : lexer(lex) {}

    // Inherited via the Subject Interface
    virtual void NotifyObservers(const Notify what) override { for (auto& obs : observers) obs->Update(root.get()); }

    // Inherited via IObserver Interface
    virtual bool ShouldRun() const override { return shouldRun; }
    virtual void SetToRun()        override { shouldRun = true; }
    virtual void Update()          override;
    virtual void Reset()           override;
};