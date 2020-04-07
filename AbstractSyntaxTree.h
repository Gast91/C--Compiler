#pragma once
#include <iostream>

#include "Utility.h"
#include "Visitor.h"
#include "Token.h"

// Base Node class
class ASTNode
{
public:
    std::string parentID;
public:
    ASTNode() = default;
    virtual ~ASTNode() = default;

    // Function allowing the implementation of the visitor pattern
    virtual void Accept(ASTNodeVisitor& v) = 0;
    virtual void SetChildrenPrintID(const std::string& pID) = 0;
};

// Abstract Syntax Tree Node with one branch or leaf
class UnaryASTNode : public ASTNode
{
public:
    ASTNode* expr;
public:
    UnaryASTNode(ASTNode* n) noexcept : expr(n) {}
    ~UnaryASTNode() { delete expr; }

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override { expr->parentID = pID; }
};

class UnaryOperationNode : public UnaryASTNode
{
public:
    TokenPair op;
public:
    UnaryOperationNode(TokenPair t, ASTNode* n) noexcept : UnaryASTNode(n), op(t) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override { expr->parentID = pID; }
};

// Abstract Syntax Tree Node with two branches or leaves
class BinaryASTNode : public ASTNode
{
public:
    ASTNode*   left;
    ASTNode*   right;
    TokenPair  op;
public:
    BinaryASTNode(ASTNode* l, TokenPair o, ASTNode* r) noexcept : left(l), op(o), right(r) {}
    ~BinaryASTNode()
    {
        delete left;
        delete right;
    }

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override { left->parentID = pID; right->parentID = pID; }
};

// Node representing a number(integer) literal
class IntegerNode : public ASTNode       // this doesnt have a token anymore - does it need one for the future?
{
public:
    int value;
public:
    IntegerNode(const std::string& val) : value(std::stoi(val)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) noexcept override { /* No children */ }
};

// Node representing an identifier
class IdentifierNode : public ASTNode
{
public:
    Token type;
    const std::string name;
    const std::string lineNo;
public:
    IdentifierNode(const std::string& n, const std::string& line, const Token t = Token::UNKNOWN) : name(n), lineNo(line), type(t) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) noexcept override { /* No children */ }
};

// Node representing a binary operation (Addition, Subtraction, Multiplication or Division)
class BinaryOperationNode : public BinaryASTNode
{
public:
    BinaryOperationNode(ASTNode* l, TokenPair o, ASTNode* r) noexcept : BinaryASTNode(l, o, r) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
};

class ConditionNode : public BinaryASTNode  // maybe we need it to be an actual node? that holds all conditional expr ? right now its only one binaryOP
{
public:
    ConditionNode(ASTNode* l, TokenPair& o, ASTNode* r) noexcept : BinaryASTNode(l, o, r) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
};

// Node representing an IF condition body or an ELSE_IF condition body. Part of an umbrella IfStatementNode
class IfNode : public ASTNode
{
public:
    ASTNode* condition;
    ASTNode* body;
    std::string type;     // IF or ELSE_IF
public:
    IfNode(ASTNode* cond, ASTNode* b) noexcept : condition(cond), body(b) {}
    ~IfNode()
    {
        delete condition;
        if (body) delete body;
    }

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override { condition->parentID = pID; body->parentID = pID; }
};

// Node representing a collection of an IFNode, several ELSE_IF and an ELSE CompoundStatementNode
class IfStatementNode : public ASTNode
{
public:
    std::vector<IfNode*> ifNodes;
    ASTNode* elseBody;
public:
    ~IfStatementNode()
    {
        for (const auto& ifN : ifNodes)  delete ifN;
        if (elseBody) delete elseBody;
    }

    void AddNode(IfNode* node)
    {
        ifNodes.empty() ? node->type = "IF" : node->type = "ELSEIF";
        ifNodes.push_back(node);
    }
    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override
    {
        for (const auto& ifN : ifNodes) ifN->parentID = pID;
        elseBody->parentID = pID;
    }
};

class IterationNode : public ASTNode
{
public:
    ASTNode* condition;
    ASTNode* body;
public:
    IterationNode(ASTNode* cond, ASTNode* b) noexcept : condition(cond), body(b) {}
    ~IterationNode()
    {
        delete condition;
        if (body) delete body;
    }

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override { condition->parentID = pID; body->parentID = pID; }
};

class WhileNode : public IterationNode
{
public:
    WhileNode(ASTNode* cond, ASTNode* b) noexcept : IterationNode(cond, b) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
};

class DoWhileNode : public IterationNode
{
public:
    DoWhileNode(ASTNode* cond, ASTNode* b) noexcept : IterationNode(cond, b) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
};

class CompoundStatementNode : public ASTNode
{
public:
    std::vector<ASTNode*> statements;
public:
    CompoundStatementNode() = default;
    ~CompoundStatementNode() { for (const auto& statement : statements) delete statement; }

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override { for (const auto& statement : statements) statement->parentID = pID; }

    void Push(ASTNode* statement) { statements.push_back(statement); }
};

class StatementBlockNode : public CompoundStatementNode
{
public:
    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
};

class DeclareStatementNode : public ASTNode
{
public:
    IdentifierNode* identifier;
    TokenPair type;
public:
    DeclareStatementNode(IdentifierNode* ident, TokenPair t) noexcept : identifier(ident), type(t) { identifier->type = t.second; }
    ~DeclareStatementNode() { delete identifier; }

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override {identifier->parentID = pID; }
};

class AssignStatementNode : public BinaryASTNode
{
public:
    AssignStatementNode(IdentifierNode* ident, ASTNode* expr) noexcept : BinaryASTNode(ident, { "=", Token::ASSIGN }, expr) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
};

class ReturnStatementNode : public UnaryASTNode
{
public:
    ReturnStatementNode(ASTNode* n) noexcept : UnaryASTNode(n) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override { expr->parentID = pID; }
};

class EmptyStatementNode : public ASTNode
{
public:
    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) noexcept override { /* No Children */ }
};

/*
Fix/Add:    -Store type in literals?
*/