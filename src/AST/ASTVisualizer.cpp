#include "ASTVisualizer.h"
#include "AbstractSyntaxTree.h"

ASTVisualizer::ASTVisualizer(bool console) noexcept : consoleOutput(console) {}

void ASTVisualizer::PrintAST(ASTNode& n)
{
    out.open("AST.js");    // error checking - unable to open file? also must set to completely overwrite
    // First node is the parent of all other nodes and doesnt have a parent itself, its parentID is itself
    n.parentID = GenerateJSONHeader(out, &n, "ROOT", config);  // In the future this might also be a function
    // The next node in line will look for this one's id and that is why it's value is set to its own id rather than null
    n.SetChildrenPrintID(n.parentID);
    // Recursively visit each of the tree's nodes and print it to the console
    if (consoleOutput)
    {
        std::cout << "-> Console Dump:\n";
        n.Accept(*this);
        consoleOutput = false;
    }
    else std::cout << '\n';
    // Recursively visit each of the tree's nodes and print JSON to file
    n.Accept(*this);

    // Footer of the file must be a configuration list of all the nodes and their ids
    GenerateJSONFooter(out, config);
    out.close();
    std::cout << "\n\nAST Visualisation File Successfully created\n";
}

void ASTVisualizer::Visit(ASTNode& n)       { assert(("ASTVisualizer visited base ASTNode class?!"      , false)); }
void ASTVisualizer::Visit(UnaryASTNode& n)  { assert(("ASTVisualizer visited base UnaryASTNode class?!" , false)); }
void ASTVisualizer::Visit(BinaryASTNode& n) { assert(("ASTVisualizer visited base BinaryASTNode class?!", false)); }

void ASTVisualizer::Visit(IntegerNode& n)
{
    if   (consoleOutput) std::cout << "Int: " << n.value;
    else (void)GenerateJSON(out, &n, "INT", n.parentID, std::to_string(n.value), config);
}

void ASTVisualizer::Visit(IdentifierNode& n)
{
    if    (consoleOutput) std::cout << "Ident: " << n.name;
    else  (void)GenerateJSON(out, &n, "ID", n.parentID, n.name, config);
}

void ASTVisualizer::Visit(UnaryOperationNode& n)
{
    if (consoleOutput)
    {
        std::cout << "Unary: '" << n.op.first << "' [";
        n.expr->Accept(*this);
        std::cout << "]";
    }
    else
    {
        n.SetChildrenPrintID(GenerateJSON(out, &n, "UNARY", n.parentID, n.op.first, config));
        n.expr->Accept(*this);
    }
}

void ASTVisualizer::Visit(BinaryOperationNode& n)
{
    if (consoleOutput)
    {
        std::cout << "BinOp: '" << n.op.first << "' [L: ";
        n.left->Accept(*this);
        std::cout << " R: ";
        n.right->Accept(*this);
        std::cout << "]";
    }
    else
    {
        n.SetChildrenPrintID(GenerateJSON(out, &n, "BINOP", n.parentID, n.op.first, config));
        n.left->Accept(*this);
        n.right->Accept(*this);
    }
}

void ASTVisualizer::Visit(ConditionNode& n)
{
    if (consoleOutput)
    {
        std::cout << "COND: '" << n.op.first << "' [L: ";
        n.left->Accept(*this);
        std::cout << " R: ";
        n.right->Accept(*this);
        std::cout << "]";
    }
    else
    {
        n.SetChildrenPrintID(GenerateJSON(out, &n, "COND", n.parentID, n.op.first, config));
        n.left->Accept(*this);
        n.right->Accept(*this);
    }
}

void ASTVisualizer::Visit(IfNode& n)
{
    if (consoleOutput)
    {
        std::cout << '\n' << n.type << ": [";
        n.condition->Accept(*this);
        std::cout << " BODY: ";
        // If body can be empty
        if (n.body) n.body->Accept(*this);
        std::cout << "]";
    }
    else
    {
        n.SetChildrenPrintID(GenerateJSON(out, &n, n.type.c_str(), n.parentID, n.type.c_str(), config));
        n.condition->Accept(*this);
        if (n.body) n.body->Accept(*this);
    }
}

void ASTVisualizer::Visit(IfStatementNode& n)
{
    if (consoleOutput)
    {
        std::cout << "\nIF_STATEMENT: [";
        for (const auto& ifN : n.ifNodes) ifN->Accept(*this);
        std::cout << "]";
        if (n.elseBody)
        {
            std::cout << "\nELSE: [ BODY: ";
            n.elseBody->Accept(*this);
            std::cout << "]";
        }
    }
    else
    {
        n.SetChildrenPrintID(GenerateJSON(out, &n, "_IF_", n.parentID, "_IF_", config));  // else is compound so parent automatically becomes _IF_ shows no else!
        for (const auto& ifN : n.ifNodes) ifN->Accept(*this);
        if (n.elseBody) n.elseBody->Accept(*this);
    }
}

void ASTVisualizer::Visit(IterationNode& n) { assert(("ASTVisualizer visited base IterationNode class?!", false)); }
void ASTVisualizer::Visit(WhileNode& n)
{
    if (consoleOutput)
    {
        std::cout << "\nWHILE: " << "[";
        n.condition->Accept(*this);
        std::cout << " BODY: ";
        // While-body can be empty
        if (n.body) n.body->Accept(*this);
        std::cout << "]";
    }
    else
    {
        n.SetChildrenPrintID(GenerateJSON(out, &n, "WHILE", n.parentID, "WHILE", config));
        n.condition->Accept(*this);
        if (n.body) n.body->Accept(*this);
    }
}

void ASTVisualizer::Visit(DoWhileNode& n)
{
    if (consoleOutput)
    {
        std::cout << "\nDO: [ BODY: ";
        if (n.body) n.body->Accept(*this);
        std::cout << " WHILE: ";
        // While-body can be empty
        n.condition->Accept(*this);
        std::cout << "]";
    }
    else
    {
        n.SetChildrenPrintID(GenerateJSON(out, &n, "DO", n.parentID, "DO", config));
        if (n.body) n.body->Accept(*this);
        n.condition->Accept(*this);
    }
}

void ASTVisualizer::Visit(CompoundStatementNode& n)
{
    // Each child of the compound has this compound as a parent (but we never visualise compound statements)
    // so the parent of the children is in fact the parent of the compound node
    n.SetChildrenPrintID(n.parentID);
    for (const auto& statement : n.statements) statement->Accept(*this);
}

void ASTVisualizer::Visit(StatementBlockNode& n)
{
    // Each child of the compound has this compound as a parent (but we never visualise compound statements)
    // so the parent of the children is in fact the parent of the compound node
    n.SetChildrenPrintID(n.parentID);
    for (const auto& statement : n.statements) statement->Accept(*this);
}

void ASTVisualizer::Visit(DeclareStatementNode& n)
{
    if (consoleOutput)
    {
        std::cout << '\n' << n.type.first << " DECL: [";
        n.identifier->Accept(*this);
        std::cout << "]";
    }
    else
    {
        n.SetChildrenPrintID(GenerateJSON(out, &n, "DECL", n.parentID, n.type.first, config));
        n.identifier->Accept(*this);
    }
}

void ASTVisualizer::Visit(DeclareAssignNode& n)
{
    if (consoleOutput)
    {
        std::cout << '\n' << "DECL_ASSIGN: [L: ";
        n.left->Accept(*this);
        std::cout << " R: ";
        n.right->Accept(*this);
        std::cout << "]";
    }
    else
    {
        n.SetChildrenPrintID(GenerateJSON(out, &n, "DECL_ASSIGN", n.parentID, n.op.first, config));
        n.left->Accept(*this);
        n.right->Accept(*this);
    }
}

void ASTVisualizer::Visit(AssignStatementNode& n)
{
    if (consoleOutput)
    {
        std::cout << "\nASSIGN: '" << n.op.first << "' [L: ";
        n.left->Accept(*this);
        std::cout << " R: ";
        n.right->Accept(*this);
        std::cout << "]";
    }
    else
    {
        n.SetChildrenPrintID(GenerateJSON(out, &n, "ASSIGN", n.parentID, n.op.first, config));
        n.left->Accept(*this);
        n.right->Accept(*this);
    }
}

void ASTVisualizer::Visit(ReturnStatementNode& n)
{
    if (consoleOutput)
    {
        std::cout << "\nRETURN: [";
        n.expr->Accept(*this);
        std::cout << "]";
    }
    else
    {
        n.SetChildrenPrintID(GenerateJSON(out, &n, "RETURN", n.parentID, "RETURN", config));
        n.expr->Accept(*this);
    }
}

void ASTVisualizer::Visit(EmptyStatementNode& n) {}