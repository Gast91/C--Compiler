#include "CodeGenerator.h"
#include "AbstractSyntaxTree.h"

ThreeAddressCode CodeGenerator::instructions;
int Temporary::tempCount = 0;
int Label::labelCount = 0;

static const std::map<std::string, std::string> registers =
{
    {"_t0", "eax" },
    {"_t1", "edx" },
    {"_t2", "ebx" },
    {"_t3", "ecx" }
};


void CodeGenerator::GenerateTAC(ASTNode* n)
{
    std::cout << "Intermediate Language Representation:\nmain:\n";
    PlainVisit(n);  // Start Traversing the AST
    for (const auto& instruction : instructions)   // no string comparisons pls - how about string_view or something else - merge stuff
    {
        // if there is a second operand then the instruction represent's a full three address code (3 addresses, 2 max operands on the right)
        if (instruction.src2.has_value())
            std::cout << '\t' << instruction.dest.value().name << " = " << instruction.src1.value().name << ' ' << instruction.op.value() << ' ' << instruction.src2.value().name << ";\n";
        else if (instruction.op.value() == "=")       
            std::cout << '\t' << instruction.dest.value().name << ' ' << instruction.op.value() << ' ' << instruction.src1.value().name << ";\n";
        else if (!instruction.op.value().compare(0, 2, "If"))   
            std::cout << '\t' << instruction.op.value() << ' ' << instruction.src1.value().name << " Goto " << instruction.dest.value().name << ";\n";
        else if (instruction.op.value() == "Label")   std::cout << instruction.dest.value().name << ":\n";
        else if (instruction.op.value() == "Return")  std::cout << '\t' << instruction.op.value() << ' ' << instruction.dest.value().name << ";\n";  // if it works fix comps
        else if (instruction.op.value() == "Goto")    std::cout << '\t' << instruction.op.value() << ' ' << instruction.dest.value().name << ":\n";
        // Unary Operation Instruction
        else std::cout << '\t' << instruction.dest.value().name << " = " << instruction.op.value() << ' ' << instruction.src1.value().name << ";\n";
    }
    GenerateAssembly();
}

void CodeGenerator::GenerateAssembly()  // variable names must be their stack offset
{
    std::cout << "\nx86 Assembly??: \n";
    std::cout << "main:\n\tpush ebp\n\tmov ebp, esp\n";
    for (const auto& instruction : instructions)
    {
        auto[op, src1, src2, dest] = instruction;
        const auto operand1 = registers.find(src1.value().name) != registers.end() ? registers.at(src1.value().name) : src1.value().address;    // at the end operand 2 must be a register or address
        const auto destination = registers.find(dest.value().name) != registers.end() ? registers.at(dest.value().name) : dest.value().address; // at the end operand 2 must be a register or address
        if (src2.has_value())
        {
            const auto operand2 = registers.find(src2.value().name) != registers.end() ? registers.at(src2.value().name) : src2.value().address; // at the end operand 2 must be a register or address
            //if (!dest.value().compare(0, 2, "_t"))
            if (dest.value().type == Token::TEMPORARY)
            {
/*is this needed now?*/if (src1.value().name != dest.value().name) std::cout << "\tmov " << destination << ", " << operand1 << '\n';   // what if src1 and/or two are registers!
                if (op.value() == "*")      std::cout << "\timul " << destination << ", " << operand2 << '\n';    // multiply must always be the eax
                else if (op.value() == "+") std::cout << "\tadd "  << destination << ", " << operand2 << '\n';
                else if (op.value() == "-") std::cout << "\tsub "  << destination << ", " << operand2 << '\n';
                else if (op.value() == "/") std::cout << "\tdiv "  << destination << ", " << operand2 << '\n';
            }
            else if (!op.value().compare(0, 2, "If"));  // <---  we have taken care of the condition, so we need to compare and jump to destination depending
            else if (op.value() == "Goto") std::cout << "\tjmp" << destination << '\n';
            else if (dest.value().type == Token::LABEL) std::cout << destination << ":\n";
            //else
            //{
            //    // we still need to move to a reg fuck the opt fucked me?
            //    if (op.value() == "*")      std::cout << "\timul " << operand1 << ", " << operand2 << '\n';      // multiply must always be the eax
            //    else if (op.value() == "+") std::cout << "\tadd "  << operand1 << ", " << operand2 << '\n';
            //    else if (op.value() == "-") std::cout << "\tsub "  << operand1 << ", " << operand2 << '\n';
            //    else if (op.value() == "/") std::cout << "\tdiv "  << operand1 << ", " << operand2 << '\n';
            //    std::cout << "\tmov " << destination << ", " << operand1 << '\n';
            //}
        }
        else
        {
            std::cout << "\tmov " << destination << ", " << operand1 << '\n';   // this only for assigns and unaries so far - return for example does other things
            if (op.value() == "-") std::cout << "\tneg " << destination << '\n';
        }
    }
    std::cout << "\tpop ebp\n\tret";
}

void CodeGenerator::Visit(ASTNode& n)        { assert(("Code Generator visited base ASTNode class?!"      , false)); }
void CodeGenerator::Visit(UnaryASTNode& n)   { assert(("Code Generator visited base UnaryASTNode class?!" , false)); }
void CodeGenerator::Visit(BinaryASTNode& n)  { assert(("Code Generator visited base BinaryASTNode class?!", false)); }
// Integer and Identifier Leaf Nodes. A throwaway Quadruple is returned that effectively passes back their value or name
//void CodeGenerator::Visit(IntegerNode& n)    { Return({ std::nullopt, std::nullopt, std::nullopt, std::to_string(n.value) }); }
void CodeGenerator::Visit(IntegerNode& n) { Return({ std::nullopt, std::nullopt, std::nullopt, Operand{Token::INT_LITERAL, std::to_string(n.value), std::to_string(n.value)} }); }
//void CodeGenerator::Visit(IdentifierNode& n) { Return({ std::nullopt, std::nullopt,  std::nullopt, n.name }); }
void CodeGenerator::Visit(IdentifierNode& n) { Return({ std::nullopt, std::nullopt,  std::nullopt, Operand{Token::IDENTIFIER, n.name, " DWORD PTR [ebp" + n.offset + "]"} }); }

void CodeGenerator::Visit(UnaryOperationNode& n)
{
    instructions.push_back({ n.op.first, fetch_instr(n.expr.get()).dest, std::nullopt, Temporary::NewTemporary() });
    Return(instructions.back());
}

void CodeGenerator::Visit(BinaryOperationNode& n)
{
#ifdef OPTIMIZE_TEMPS
    const auto src1 = fetch_instr(n.left.get()).dest;
    const auto dest = Temporary::NewTemporary();
    const auto src2 = fetch_instr(n.right.get()).dest;
    instructions.push_back({ n.op.first, src1, src2, dest });
#else
    instructions.push_back({ n.op.first, fetch_instr(n.left).dest, fetch_instr(n.right).dest, Temporary::NewTemporary() });
#endif // OPTIMIZE_TEMPS
    Return(instructions.back());
}

void CodeGenerator::Visit(ConditionNode& n) { assert(("Code Generator visited ConditionNode class?!", false)); }

void CodeGenerator::Visit(IfNode& n)
{
    const auto falseLabel = Label::NewLabel();    // are all relational operators allowed? do i need more spliting or something?
    instructions.push_back({ "IfFalse", fetch_instr(n.condition.get()).dest, std::nullopt, falseLabel });
    if (n.body) PlainVisit(n.body.get());
    instructions.push_back({ "Label", std::nullopt, std::nullopt, falseLabel });
}
void CodeGenerator::Visit(IfStatementNode& n)
{
    for (const auto& ifN : n.ifNodes) PlainVisit(ifN.get());
    if (n.elseBody) PlainVisit(n.elseBody.get());
}
void CodeGenerator::Visit(IterationNode& n) { assert(("Code Generator visited base IterationNode class?!", false)); }

void CodeGenerator::Visit(WhileNode& n)
{
    const auto startLabel = Label::NewLabel();
    instructions.push_back({ "Label", std::nullopt, std::nullopt, startLabel });
    const auto endLabel = Label::NewLabel();
    instructions.push_back({ "IfFalse", fetch_instr(n.condition.get()).dest, std::nullopt, endLabel });
    if (n.body) PlainVisit(n.body.get());
    instructions.push_back({ "Goto", std::nullopt, std::nullopt, startLabel });
    instructions.push_back({ "Label", std::nullopt, std::nullopt, endLabel });
}

void CodeGenerator::Visit(DoWhileNode& n)
{
    const auto startLabel = Label::NewLabel();
    instructions.push_back({ "Label", std::nullopt, std::nullopt, startLabel });
    if (n.body) PlainVisit(n.body.get());
    instructions.push_back({ "If", fetch_instr(n.condition.get()).dest, std::nullopt, startLabel });
}

void CodeGenerator::Visit(CompoundStatementNode& n)
{
    for (const auto& statement : n.statements) PlainVisit(statement.get());
    // the way it is set up here, generation will start here, maybe have a ProgramEntryNode?  also label main:
    // for setting up stack etc? for exit as well - wont work atm - hack atm is generate
}

void CodeGenerator::Visit(StatementBlockNode& n) { for (const auto& statement : n.statements) PlainVisit(statement.get()); }

void CodeGenerator::Visit(DeclareStatementNode& n)
{
    // it will need to do type checking also (in the future) and tell code generator size of var? here or in semantic analysis?
    Return(GetValue(n.identifier.get()));  // might change but for now this returns its identifier, used only for declare-assign
}

void CodeGenerator::ProcessAssignment(const BinaryASTNode& n)
{
//#ifdef OPTIMIZE_TEMPS
//    // Get the instruction from the expression (temporary with expression, literal or identifier) from the right
//    const auto src2 = fetch_instr(n.right.get());
//    // If its an operation (not just an identifier or literal)
//    if (src2.op.has_value())
//    {
//        // Remove the previous instruction
//        instructions.pop_back();
//        // And reform it as a direct assignment of the operation to your left operand
//        instructions.push_back({ src2.op, src2.src1, src2.src2, GetValue(n.left.get()).dest });
//    }
//    // Just a literal or identifier, assign it to your left
//    else instructions.push_back({ n.op.first, src2.dest, std::nullopt, GetValue(n.left.get()).dest });
//#else
    // Assign the expression to your left
    instructions.push_back({ n.op.first, fetch_instr(n.right.get()).dest, std::nullopt, GetValue(n.left.get()).dest });
//#endif // OPTIMIZE_TEMPS
}

void CodeGenerator::Visit(DeclareAssignNode& n)   { ProcessAssignment(n); }

void CodeGenerator::Visit(AssignStatementNode& n) {  ProcessAssignment(n); }

void CodeGenerator::Visit(ReturnStatementNode& n)
{
    instructions.push_back({ "Return", std::nullopt, std::nullopt, GetValue(n.expr.get()).dest });
    // we are done here we must jump to the end or something
}

void CodeGenerator::Visit(EmptyStatementNode& n) {}

// TODO:
/*
    -Fix/Add Nodes into the ast to accomodate main/entry point - potentially more? 
    -Rename+cpp+h to IRGenerator || IRCGenerator || CodeGeneratorIR -- Last will be AssemblyGenerator(not a visitor this time?)

    check asserts if work - dont care?
    need beginfunc and endfunc, jump for return to the end and allocating space at begin func?

    -CHECK STD::VISITOR-VARIANT - nah
    https://web.stanford.edu/class/archive/cs/cs143/cs143.1128/handouts/240%20TAC%20Examples.pdf page 7-8 for reduced temporaries
*/