#include "CodeGenerator.h"
#include "AbstractSyntaxTree.h"

ThreeAddressCode CodeGenerator::instructions;
int Temporary::tempCount = 0;
int Label::labelCount = 0;

static const std::map<std::string, std::string> asmLookup =
{
//------Arithm Op--------
    {"+"  , "\tadd "  },
    {"-"  , "\tsub "  },        // what about negate
    {"/"  , "\tdiv "  },
    {"*"  , "\timul " },
//------Rel Oper---------
    {">"  , "\tjle "  },        // Note the inversion
    {"<"  , "\tjge "  },
    {">=" , "\tjl "   },
    {"<=" , "\tjg "   },
    {"!=" , "\tje "   },
    {"==" , "\tjne "  },
//------Flow Control-----
    {"Goto", "\tjmp " },
//------Registers--------
    {"_t0", "eax"     },
    {"_t1", "edx"     },
    {"_t2", "ebx"     },
    {"_t3", "ecx"     }
};

void CodeGenerator::GenerateTAC(ASTNode* n)
{
    std::cout << "Intermediate Language Representation:\nmain:\n";
    PlainVisit(n);  // Start Traversing the AST
    for (auto& [op, src1, src2, dest] : instructions)
    {
        // Presence of second operand indicates a full three address code instruction
        if (src2)                            std::cout << '\t' << dest->name << " = " << src1->name << ' ' << op->value << ' ' << src2->name << ";\n";
        else if (op->type == CmdType::COPY)  std::cout << '\t' << dest->name << ' '   << op->value  << ' ' << src1->name << ";\n";
        else if (op->type == CmdType::IF)    std::cout << '\t' << op->value  << ' '   << src1->name << " Goto " << dest->name << ";\n";
        else if (op->type == CmdType::LABEL) std::cout << dest->name << ":\n";
        else if (op->type == CmdType::RET 
              || op->type == CmdType::GOTO)  std::cout << '\t' << op->value << ' ' << dest->name << ";\n";  // if it works fix comps
        // Unary Operation Instruction
        else std::cout << '\t' << dest->name << " = " << op->value << ' ' << src1->name << ";\n";
    }
    GenerateAssembly();
}

void CodeGenerator::GenerateAssembly()
{
    std::cout << "\nx86 Assembly: \n";
    //std::cout << "main:\n\tpush ebp\n\tmov ebp, esp\n";  // No point atm since we dont have multiple functions, the way this is handled will change at the future
    for (auto& [op, src1, src2, dest] : instructions)
    {
        const auto operand1    = asmLookup.find(src1->name) != asmLookup.end() ? asmLookup.at(src1->name) : src1->address; // at the end operand 2 must be a register or address
        const auto destination = asmLookup.find(dest->name) != asmLookup.end() ? asmLookup.at(dest->name) : dest->address; // at the end operand 2 must be a register or address
        if (src2)
        {
            const auto operand2 = asmLookup.find(src2->name) != asmLookup.end() ? asmLookup.at(src2->name) : src2->address; // at the end operand 2 must be a register or address
            if (dest->type == CmdType::REG)  // should be the operator that decides what this is?
            {
                if (operand1 != destination) std::cout << "\tmov " << destination << ", " << operand1 << '\n';
                std::cout << asmLookup.at(op->value) << destination << ", " << operand2 << '\n';                 // multiply must always be in the eax| weird rules for DIV also
            }
            else if (op->type == CmdType::IF);  // <---  comparisons and jmps happen at each logical expression? if should be processed, increment the label count
            else if (op->type == CmdType::GOTO)  std::cout << asmLookup.at("Goto") << destination << '\n';
            else if (op->type == CmdType::LABEL) std::cout << destination << ":\n";
        }
        else
        {
            std::cout << "\tmov " << destination << ", " << operand1 << '\n';   // this only for assigns and unaries so far - return for example does other things
            if (op->value == "-") std::cout << "\tneg " << destination << '\n';
        }
    }
    //std::cout << "\tpop ebp\n\tret";  // No point atm since we dont have multiple functions, the way this is handled will change at the future
}

void CodeGenerator::Visit(ASTNode& n)        { assert(("Code Generator visited base ASTNode class?!"      , false)); }
void CodeGenerator::Visit(UnaryASTNode& n)   { assert(("Code Generator visited base UnaryASTNode class?!" , false)); }
void CodeGenerator::Visit(BinaryASTNode& n)  { assert(("Code Generator visited base BinaryASTNode class?!", false)); }
// Integer and Identifier Leaf Nodes. A throwaway Quadruple is returned that effectively passes back their value or name
void CodeGenerator::Visit(IntegerNode& n) { Return({ std::nullopt, std::nullopt, std::nullopt, Operand{CmdType::NONE, std::to_string(n.value), std::to_string(n.value)} }); }
void CodeGenerator::Visit(IdentifierNode& n) { Return({ std::nullopt, std::nullopt,  std::nullopt, Operand{CmdType::NONE, n.name, "DWORD [ebp" + n.offset + "]"} }); }

void CodeGenerator::Visit(UnaryOperationNode& n)
{
    instructions.push_back({ Command{n.op.first, CmdType::UNARY }, fetch_instr(n.expr.get()).dest, std::nullopt, Temporary::NewTemporary() });
    Return(instructions.back());
}

void CodeGenerator::Visit(BinaryOperationNode& n)
{
#ifdef OPTIMIZE_TEMPS
    const auto src1 = fetch_instr(n.left.get()).dest;
    const auto dest = Temporary::NewTemporary();
    const auto src2 = fetch_instr(n.right.get()).dest;
    instructions.push_back({ Command{n.op.first, CmdType::ARITHM }, src1, src2, dest });
#else
    instructions.push_back({ Command{n.op.first, CmdType::ARITHM }, fetch_instr(n.left).dest, fetch_instr(n.right).dest, Temporary::NewTemporary() });
#endif // OPTIMIZE_TEMPS
    Return(instructions.back());
}

void CodeGenerator::Visit(ConditionNode& n) 
{ 
#ifdef OPTIMIZE_TEMPS
    const auto src1 = fetch_instr(n.left.get()).dest;
    const auto dest = Temporary::NewTemporary();
    const auto src2 = fetch_instr(n.right.get()).dest;
    instructions.push_back({ Command{n.op.first, CmdType::RELAT }, src1, src2, dest });
#else
instructions.push_back({ Command{n.op.first, CmdType::RELAT }, fetch_instr(n.left).dest, fetch_instr(n.right).dest, Temporary::NewTemporary() });
#endif // OPTIMIZE_TEMPS
Return(instructions.back());
    //assert(("Code Generator visited ConditionNode class?!", false));
}

void CodeGenerator::Visit(IfNode& n)
{
    const auto falseLabel = Label::NewLabel();
    instructions.push_back({ Command{"IfFalse", CmdType::IF }, fetch_instr(n.condition.get()).dest, std::nullopt, falseLabel });
    if (n.body) PlainVisit(n.body.get());
    instructions.push_back({ Command{"Label", CmdType::LABEL }, std::nullopt, std::nullopt, falseLabel });
    
    //labels.push_back({ "", falseLabel.name });  // ???
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
    instructions.push_back({ Command{"Label", CmdType::LABEL }, std::nullopt, std::nullopt, startLabel });
    const auto endLabel = Label::NewLabel();
    instructions.push_back({ Command{"IfFalse", CmdType::IF }, fetch_instr(n.condition.get()).dest, std::nullopt, endLabel });
    if (n.body) PlainVisit(n.body.get());
    instructions.push_back({ Command{"Goto", CmdType::GOTO }, std::nullopt, std::nullopt, startLabel });
    instructions.push_back({ Command{"Label", CmdType::LABEL }, std::nullopt, std::nullopt, endLabel });
}

void CodeGenerator::Visit(DoWhileNode& n)
{
    const auto startLabel = Label::NewLabel();
    instructions.push_back({ Command{"Label", CmdType::LABEL }, std::nullopt, std::nullopt, startLabel });
    if (n.body) PlainVisit(n.body.get());
    instructions.push_back({ Command{"If", CmdType::IF }, fetch_instr(n.condition.get()).dest, std::nullopt, startLabel });
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
    instructions.push_back({ Command{ n.op.first, CmdType::COPY }, fetch_instr(n.right.get()).dest, std::nullopt, GetValue(n.left.get()).dest });
//#endif // OPTIMIZE_TEMPS
}

void CodeGenerator::Visit(DeclareAssignNode& n)   { ProcessAssignment(n); }

void CodeGenerator::Visit(AssignStatementNode& n) {  ProcessAssignment(n); }

void CodeGenerator::Visit(ReturnStatementNode& n)
{
    instructions.push_back({ Command{ "Return", CmdType::RET }, std::nullopt, std::nullopt, GetValue(n.expr.get()).dest });
    // we are done here we must jump to the end or something
}

void CodeGenerator::Visit(EmptyStatementNode& n) {}

// TODO:
/*
    -Fix/Add Nodes into the ast to accomodate main/entry point - potentially more? 
    -Rename+cpp+h to IRGenerator || IRCGenerator || CodeGeneratorIR -- Last will be AssemblyGenerator(not a visitor this time?)

    need beginfunc and endfunc, jump for return to the end and allocating space at begin func?

    -CHECK STD::VISITOR-VARIANT - nah
    https://web.stanford.edu/class/archive/cs/cs143/cs143.1128/handouts/240%20TAC%20Examples.pdf page 7-8 for reduced temporaries
*/