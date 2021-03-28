#pragma once
#include <memory>

#include "../AST/Visitor.h"
#include "../Lexer/Token.h"

// Template Alias for node ptrs
template<typename T>
using UnqPtr = std::unique_ptr<T>;

// Base Node class
class ASTNode
{
public:
    std::string parentID;
public:
    ASTNode() = default;
    ASTNode(const ASTNode&) = default;
    virtual ~ASTNode() = default;
    ASTNode(ASTNode&&) = default;
    ASTNode& operator=(const ASTNode&) = default;
    ASTNode& operator=(ASTNode&&) = default;

    // To allow a class implementing the visitor pattern to visit this node
    virtual void Accept(ASTNodeVisitor& v) = 0;
    // Sets each child's parentID to the current node's id (for AST visualization)
    virtual void SetChildrenPrintID(const std::string& pID) = 0;
};

// Abstract Syntax Tree Node with one branch or leaf
class UnaryASTNode : public ASTNode
{
public:
    UnqPtr<ASTNode> expr;
public:
    UnaryASTNode(UnqPtr<ASTNode> n) noexcept : expr(std::move(n)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override { expr->parentID = pID; }
};

class UnaryOperationNode : public UnaryASTNode
{
public:
    Token op;
public:
    UnaryOperationNode(const Token& t, UnqPtr<ASTNode> n) noexcept : UnaryASTNode(std::move(n)), op(t) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override { expr->parentID = pID; }
};

// Abstract Syntax Tree Node with two branches or leaves
class BinaryASTNode : public ASTNode
{
public:
    UnqPtr<ASTNode> left;
    UnqPtr<ASTNode> right;
    Token op;
public:
    BinaryASTNode(UnqPtr<ASTNode> l, const Token& o, UnqPtr<ASTNode> r) noexcept : left(std::move(l)), op(o), right(std::move(r)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override { left->parentID = pID; right->parentID = pID; }
};

// Node representing a number(integer) literal
class IntegerNode : public ASTNode       // this doesnt have a token anymore - does it need one for the future?
{
public:
    int value;  // should this just be a string for ease?
public:
    IntegerNode(const std::string& val) : value(std::stoi(val)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) noexcept override { /* No children */ }
};

// Node representing an identifier
class IdentifierNode : public ASTNode
{
public:
    Token token;
    std::string offset;
public:
    IdentifierNode(const Token& tok) : token(tok) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) noexcept override { /* No children */ }
};

// Node representing a binary operation
class BinaryOperationNode : public BinaryASTNode
{
public:
    BinaryOperationNode(UnqPtr<ASTNode> l, const Token& o, UnqPtr<ASTNode> r) noexcept : BinaryASTNode(std::move(l), o, std::move(r)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
};

class ConditionNode : public BinaryASTNode
{
public:
    ConditionNode(UnqPtr<ASTNode> l, const Token& o, UnqPtr<ASTNode> r) noexcept : BinaryASTNode(std::move(l), o, std::move(r)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
};

// Node representing an IF condition body or an ELSE_IF condition body. Part of an umbrella IfStatementNode
class IfNode : public ASTNode
{
public:
    UnqPtr<ASTNode> condition;
    UnqPtr<ASTNode> body;
    std::string type;  // IF or ELSE_IF used only for visualization
    std::string parentEndLabel;
public:
    IfNode(UnqPtr<ASTNode> b, UnqPtr<ASTNode> cond) noexcept : body(std::move(b)),  condition(std::move(cond)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override { condition->parentID = pID; body->parentID = pID; }
};

// Node representing a collection of an IFNode, several ELSE_IF and an ELSE CompoundStatementNode
class IfStatementNode : public ASTNode
{
public:
    std::vector<UnqPtr<IfNode>> ifNodes;
    UnqPtr<ASTNode> elseBody;
public:

    void AddNode(UnqPtr<IfNode> node)
    {
        ifNodes.empty() ? node->type = "IF" : node->type = "ELSEIF";
        ifNodes.push_back(std::move(node));
    }
    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override
    {
        for (const auto& ifN : ifNodes) ifN->parentID = pID;
        if (elseBody) elseBody->parentID = pID;
    }
};

class IterationNode : public ASTNode
{
public:
    UnqPtr<ASTNode> condition;
    UnqPtr<ASTNode> body;
public:
    IterationNode(UnqPtr<ASTNode> cond, UnqPtr<ASTNode> b) noexcept : condition(std::move(cond)), body(std::move(b)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override { condition->parentID = pID; body->parentID = pID; }
};

class WhileNode : public IterationNode
{
public:
    WhileNode(UnqPtr<ASTNode> cond, UnqPtr<ASTNode> b) noexcept : IterationNode(std::move(cond), std::move(b)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
};

class DoWhileNode : public IterationNode
{
public:
    DoWhileNode(UnqPtr<ASTNode> cond, UnqPtr<ASTNode> b) noexcept : IterationNode(std::move(cond), std::move(b)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
};

class CompoundStatementNode : public ASTNode
{
public:
    std::vector<UnqPtr<ASTNode>> statements;
public:
    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override { for (const auto& statement : statements) statement->parentID = pID; }

    void Push(UnqPtr<ASTNode> statement) { statements.push_back(std::move(statement)); }
};

class StatementBlockNode : public CompoundStatementNode
{
public:
    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
};

class DeclareStatementNode : public ASTNode
{
public:
    UnqPtr<IdentifierNode> identifier;
    Token type;
public:
    DeclareStatementNode(UnqPtr<IdentifierNode> ident, const Token& t) noexcept : identifier(std::move(ident)), type(t) { identifier->token.type = t.type; }

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override {identifier->parentID = pID; }
};

class DeclareAssignNode : public BinaryASTNode
{
public:
    DeclareAssignNode(UnqPtr<DeclareStatementNode> decl, const Token& o, UnqPtr<ASTNode> expr) noexcept : BinaryASTNode(std::move(decl), o, std::move(expr)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
};

class AssignStatementNode : public BinaryASTNode
{
public:
    AssignStatementNode(UnqPtr<IdentifierNode> ident, const Token& o, UnqPtr<ASTNode> expr) noexcept : BinaryASTNode(std::move(ident), o, std::move(expr)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
};

class ReturnStatementNode : public UnaryASTNode
{
public:
    ReturnStatementNode(UnqPtr<ASTNode> n) noexcept : UnaryASTNode(std::move(n)) {}

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