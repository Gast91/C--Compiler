#pragma once
#include <cassert>

// Forward Declarations
class ASTNode;
class UnaryASTNode;
class BinaryASTNode;
class IntegerNode;
class IdentifierNode;
class UnaryOperationNode;
class BinaryOperationNode;
class ConditionNode;
class IfNode;
class IterationNode;
class WhileNode;
class DoWhileNode;
class StatementBlockNode;
class CompoundStatementNode;
class DeclareStatementNode;
class AssignStatementNode;
class ReturnStatementNode;
class EmptyStatementNode;

// Pure Abstract Class/Interface that all classes that "walk" the AST should inherit from
// Based on the Visitor Pattern
// Each member of the ASTNode family defines an accept function that accepts the visitor class
// and calls its visit methods based on the ASTNode class that called it.
class ASTNodeVisitor
{
public:
    virtual ~ASTNodeVisitor() = default;
    virtual void Visit(ASTNode& n) = 0;
    virtual void Visit(UnaryASTNode& n) = 0;
    virtual void Visit(BinaryASTNode& n) = 0;
    virtual void Visit(IntegerNode& n) = 0;
    virtual void Visit(IdentifierNode& n) = 0;
    virtual void Visit(UnaryOperationNode& n) = 0;
    virtual void Visit(BinaryOperationNode& n) = 0;
    virtual void Visit(ConditionNode& n) = 0;
    virtual void Visit(IfNode& n) = 0;
    virtual void Visit(IterationNode& n) = 0;
    virtual void Visit(WhileNode& n) = 0;
    virtual void Visit(DoWhileNode& n) = 0;
    virtual void Visit(StatementBlockNode& n) = 0;
    virtual void Visit(CompoundStatementNode& n) = 0;
    virtual void Visit(DeclareStatementNode& n) = 0;
    virtual void Visit(AssignStatementNode& n) = 0;
    virtual void Visit(ReturnStatementNode& n) = 0;
    virtual void Visit(EmptyStatementNode& n) = 0;
};

// Curiously Recurring Template Pattern - https://www.codeproject.com/Tips/1018315/Visitor-with-the-Return-Value
template <typename VisitorImpl, typename VisitablePtr, typename ResultType>
class ValueGetter
{
private:
    ResultType value;
public:
    void PlainVisit(VisitablePtr n)
    {
        VisitorImpl vis;
        n->Accept(vis);
    }
    static ResultType GetValue(VisitablePtr n)
    {
        VisitorImpl vis;
        n->Accept(vis);
        return vis.value;
    }
    void Return(ResultType val)
    {
        value = val;
    }
};

// Inheriting Classes:
// ASTVisualizer
// SemanticAnalyser
// Code Generator
// Interpreter (NOT BEING IMPLEMENTED)