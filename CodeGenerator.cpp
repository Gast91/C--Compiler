#include "CodeGenerator.h"
#include "AbstractSyntaxTree.h"

ThreeAddressCode CodeGenerator::instructions;
int Temporary::tempCount = 0;
int Label::labelCount = 0;

void CodeGenerator::GenerateAssembly(ASTNode* n)
{
    // set up stack/main;
    /*std::cout << "\npush rbp\nmov rbp, rsp\n";
    n->Accept(*this);
    std::cout << "pop rbp\nret\n";*/
    std::cout << "Intermediate Language Representation:\nmain:\n";
    PlainVisit(n);  // Start Traversing the AST
    for (const auto& instruction : instructions)   // no string comparisons pls - how about string_view or something else - merge stuff
    {
        // if there is a second operand then the instruction represent's a full three address code (3 addresses, 2 max operands on the right)
        if (instruction.src2.has_value())
            std::cout << '\t' << instruction.dest.value() << " = " << instruction.src1.value() << ' ' << instruction.op.value() << ' ' << instruction.src2.value() << ";\n";
        else if (instruction.op.value() == "=")       
            std::cout << '\t' << instruction.dest.value() << ' ' << instruction.op.value() << ' ' << instruction.src1.value() << ";\n";
        else if (!instruction.op.value().compare(0, 2, "If"))   
            std::cout << '\t' << instruction.op.value() << ' ' << instruction.src1.value() << " Goto " << instruction.dest.value() << ";\n";
        else if (instruction.op.value() == "Label")   std::cout << instruction.dest.value() << ":\n";
        else if (instruction.op.value() == "Return")  std::cout << '\t' << instruction.op.value() << ' ' << instruction.dest.value() << ";\n";
        else if (instruction.op.value() == "Goto")    std::cout << '\t' << instruction.op.value() << ' ' << instruction.dest.value() << ":\n";
        // Unary Instruction
        else std::cout << '\t' << instruction.dest.value() << " = " << instruction.op.value() << ' ' << instruction.src1.value() << ";\n";
    }
}

void CodeGenerator::Visit(ASTNode& n)        { assert(("Code Generator visited base ASTNode class?!"      , false)); }
void CodeGenerator::Visit(UnaryASTNode& n)   { assert(("Code Generator visited base UnaryASTNode class?!" , false)); }
void CodeGenerator::Visit(BinaryASTNode& n)  { assert(("Code Generator visited base BinaryASTNode class?!", false)); }
// Integer and Identifier Leaf Nodes. A throwaway Quadruple is returned that effectively passes back their value or name
void CodeGenerator::Visit(IntegerNode& n)    { Return({ std::nullopt, std::nullopt, std::nullopt, std::to_string(n.value) }); }
void CodeGenerator::Visit(IdentifierNode& n) { Return({ std::nullopt, std::nullopt,  std::nullopt, n.name }); }

void CodeGenerator::Visit(UnaryOperationNode& n)
{
    instructions.push_back({ n.op.first, obtain_source(n.expr), std::nullopt, Temporary::NewTemporary() });
    Return(instructions.back());
}

void CodeGenerator::Visit(BinaryOperationNode& n)
{
#ifdef  OPTIMIZE
    const auto src1 = obtain_source(n.left);
    const auto dest = Temporary::NewTemporary();
    const auto src2 = obtain_source(n.right);
#else
    const auto src1 = obtain_source(n.left);
    const auto src2 = obtain_source(n.right);
    const auto dest = Temporary::NewTemporary();
#endif //  OPTIMIZE
    instructions.push_back({ n.op.first, src1, src2, dest });
    Return(instructions.back());
}

void CodeGenerator::Visit(ConditionNode& n) { assert(("Code Generator visited ConditionNode class?!", false)); }

void CodeGenerator::Visit(IfNode& n)
{
    const auto falseLabel = Label::NewLabel();    // are all relational operators allowed? do i need more spliting or something?
    instructions.push_back({ "IfFalse", obtain_source(n.condition), std::nullopt, falseLabel });
    if (n.body) PlainVisit(n.body);
    instructions.push_back({ "Label", std::nullopt, std::nullopt, falseLabel });
}
void CodeGenerator::Visit(IfStatementNode& n)
{
    for (const auto& ifN : n.ifNodes) PlainVisit(ifN);
    if (n.elseBody) PlainVisit(n.elseBody);
}
void CodeGenerator::Visit(IterationNode& n) { assert(("Code Generator visited base IterationNode class?!", false)); }

void CodeGenerator::Visit(WhileNode& n)
{
    const auto startLabel = Label::NewLabel();
    instructions.push_back({ "Label", std::nullopt, std::nullopt, startLabel });
    const auto endLabel = Label::NewLabel();
    instructions.push_back({ "IfFalse", obtain_source(n.condition), std::nullopt, endLabel });
    if (n.body) PlainVisit(n.body);
    instructions.push_back({ "Goto", std::nullopt, std::nullopt, startLabel });
    instructions.push_back({ "Label", std::nullopt, std::nullopt, endLabel });
}

void CodeGenerator::Visit(DoWhileNode& n)
{
    const auto startLabel = Label::NewLabel();
    instructions.push_back({ "Label", std::nullopt, std::nullopt, startLabel });
    if (n.body) PlainVisit(n.body);
    instructions.push_back({ "If", obtain_source(n.condition), std::nullopt, startLabel });
}

void CodeGenerator::Visit(CompoundStatementNode& n)
{
    for (const auto& statement : n.statements) PlainVisit(statement);
    // the way it is set up here, generation will start here, maybe have a ProgramEntryNode?  also label main:
    // for setting up stack etc? for exit as well - wont work atm - hack atm is generate
}

void CodeGenerator::Visit(StatementBlockNode& n) { for (const auto& statement : n.statements) PlainVisit(statement); }

void CodeGenerator::Visit(DeclareStatementNode& n)
{
    // it will need to do type checking also (in the future) and tell code generator size of var? here or in semantic analysis?
    Return(GetValue(n.identifier));  // might change but for now this returns its identifier, used only for declare-assign
}

void CodeGenerator::Visit(DeclareAssignNode& n)
{
    // Get the temporary or literal or identifier from the expression to the right and push an assignment
    // instruction using the identifier from the left as the destination
    instructions.push_back({ n.op.first, obtain_source(n.right), std::nullopt, GetValue(n.left).dest });
}

void CodeGenerator::Visit(AssignStatementNode& n)
{
    instructions.push_back({ n.op.first, obtain_source(n.right), std::nullopt, GetValue(n.left).dest });
}

void CodeGenerator::Visit(ReturnStatementNode& n)
{
    instructions.push_back({ "Return", std::nullopt, std::nullopt, GetValue(n.expr).dest });
    // we are done here we must jump to the end or something
}

void CodeGenerator::Visit(EmptyStatementNode& n) {}

// TODO:
/*
    -Fix/Add Nodes into the ast to accomodate main/entry point - potentially more? 
    -Rename+cpp+h to IRGenerator || IRCGenerator || CodeGeneratorIR -- Last will be AssemblyGenerator(not a visitor this time?)

    check asserts if work - dont care?
    temporaries now get recycled but it can even be better by avoiding some assignments
    need beginfunc and endfunc, jump for return to the end and allocating space at begin func?

    -CHECK STD::VISITOR-VARIANT - nah
    https://web.stanford.edu/class/archive/cs/cs143/cs143.1128/handouts/240%20TAC%20Examples.pdf page 7-8 for reduced temporaries
*/