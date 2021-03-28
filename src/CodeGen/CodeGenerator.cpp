#include <map>
#include <sstream>

#include "CodeGenerator.h"
#include "../AST/AbstractSyntaxTree.h"
#include "../Util/Logger.h"

std::vector<Quadruples> CodeGenerator::instructions;
int Temporary::tempCount = 0;
int Label::labelCount = 0;
int Label::nextCmpLabel = 0;
std::vector<std::string> Label::cmpJmpLabels;

static const std::map<std::string, std::string> asmLookup =
{
//------Arithm Op--------
    {"+"  , "\tadd "  },
    {"-"  , "\tsub "  },
    {"/"  , "\tdiv "  },
    {"*"  , "\timul " },
//------Rel Oper---------
    {">"  , "\tjg "   },
    {"<"  , "\tjl "   },
    {">=" , "\tjge "  },
    {"<=" , "\tjle "  },
    {"!=" , "\tjne "  },
    {"==" , "\tje "   },
//------Flow Control-----
    {"Goto", "\tjmp " },
//------Registers--------
    {"_t0", "eax"     },
    {"_t1", "edx"     },
    {"_t2", "ebx"     },
    {"_t3", "ecx"     }
};

void CodeGenerator::Update()
{
    if (!shouldRun || !root || !semSuccess) return;

    Reset();

    try // ??
    {
        GenerateTAC();
        GenerateAssembly();
    }
    catch (const std::exception& ex) { Logger::Error("[CODEGEN ERROR]: Unsupported Operation {}", ex.what()); }

    shouldRun = false;
}

void CodeGenerator::Reset()
{
    instructions.clear();
    tac.str("");
    tac.clear();
    x86.str("");
    x86.clear();
}

void CodeGenerator::GenerateTAC()
{
    PlainVisit(root);  // Start Traversing the AST

    if (instructions.empty()) { Logger::Info("No Intermediate Code Generated.\n"); return; }
    else Logger::Info("Intermediate Language Representation Generated\n");

    tac << "main:\n";

    std::vector<Command> cmp;
    unsigned int cmpIndex = 0;
    for (auto& [op, src1, src2, dest] : instructions)
    {
        // Presence of second operand indicates a full three address code instruction
        if (src2)                            tac << '\t' << dest->name << " = " << src1->name << ' ' << op->value << ' ' << src2->name << ";\n";
        else if (op->type == CmdType::COPY)  tac << '\t' << dest->name << ' '   << op->value  << ' ' << src1->name << ";\n";
        else if (op->type == CmdType::IF)
        {
            // Add the jump label of this control flow statement to a list so that
            // each subsequent relational statement that depends to it can access it. -Wont work for multiple conditions probably??
            for (auto i = cmpIndex; i < cmp.size(); ++i) Label::AddCmpLabel(dest->name);  ++cmpIndex;
            tac << '\t' << op->value << ' ' << src1->name << " Goto " << dest->name << ";\n";
        }
        else if (op->type == CmdType::LABEL) tac << dest->name << ":\n";
        else if (op->type == CmdType::RET 
              || op->type == CmdType::GOTO)  tac << '\t' << op->value << ' ' << dest->name << ";\n";
        // Unaries
        else tac << '\t' << dest->name << " = " << op->value << ' ' << src1->name << ";\n";

        if (op->type == CmdType::RELAT) cmp.push_back(Command{ dest->name, op->type });
    }
}

void CodeGenerator::GenerateAssembly()
{
    if (instructions.empty()) { Logger::Info("No assembly generated.\n"); return; }
    else Logger::Info("'Assembly' Generated\n");

    x86 << "main:\n";
    for (auto& [op, src1, src2, dest] : instructions)
    {
        const auto destination = asmLookup.find(dest->name) != asmLookup.end() ? asmLookup.at(dest->name) : dest->address;
        if (op->type == CmdType::GOTO)       x86 << asmLookup.at("Goto") << destination << '\n';
        else if (op->type == CmdType::LABEL) x86 << destination << ":\n";
        else if (op->type == CmdType::IF);   // We processed condition(s) for this control flow, no need to do anything
        else if (op->type == CmdType::RET)
        {
            x86 << "\tmov eax, " << destination << '\n'; // EAX will always have the return value
            x86 << asmLookup.at("Goto") << "_END\n";     // Jump to the end label, since return might have been nested somewhere
        }
        else if (const auto operand1 = asmLookup.find(src1->name) != asmLookup.end() ? asmLookup.at(src1->name) : src1->address; src1 && !src2)
        {
            x86 << "\tmov " << destination << ", " << operand1 << '\n';
            if (op->type == CmdType::UNARY) x86 << "\tneg " << destination << '\n';
        }
        else if (src2)
        {
            const auto operand2 = asmLookup.find(src2->name) != asmLookup.end() ? asmLookup.at(src2->name) : src2->address;
            if (op->type == CmdType::RELAT)
            {
                if (operand1 != destination) x86 << "\tmov " << destination << ", " << operand1 << '\n';
                x86 << "\tcmp " << destination << ", " << operand2 << '\n';
                x86 << ReverseOp(op->value) << ' ' << Label::GetCmpLabel() << '\n';
            }
            else if (op->type == CmdType::LOG) x86 << "\t;Multiple conditions with operators \"&&\" and \"||\" are not fully supported. There might be errors\n";
            else if (dest->type == CmdType::REG)
            {
                if (operand1 != destination) x86 << "\tmov " << destination << ", " << operand1 << '\n';
                if (const auto opCode = asmLookup.at(op->value); opCode == "\timul " && destination == "eax")
                    x86 << opCode << ' ' << operand2 << '\n';
                else if (opCode == "\timul " && destination != "eax")
                    x86 << opCode << destination << ", " << operand2 << " ;Note that \"imul reg, ... \" is not valid, eax is always the implicit register for imul\n";
                else
                    x86 << opCode << destination << ", " << operand2 << '\n';
            }
        }
    }
    x86 << "_END:\n"; // Final label that all return statements jump to - will need to change when functions are introduced
}

const std::string CodeGenerator::ReverseOp(const std::string& op) const
{
    if (op == ">")       return asmLookup.at("<=");
    else if (op == "<")  return asmLookup.at(">=");
    else if (op == ">=") return asmLookup.at("<");
    else if (op == "<=") return asmLookup.at(">");
    else if (op == "!=") return asmLookup.at("==");
    else if (op == "==") return asmLookup.at("!=");
    else                 return op;
}

void CodeGenerator::Visit(ASTNode& n)        { assert(("Code Generator visited base ASTNode class?!"      , false)); }
void CodeGenerator::Visit(UnaryASTNode& n)   { assert(("Code Generator visited base UnaryASTNode class?!" , false)); }
void CodeGenerator::Visit(BinaryASTNode& n)  { assert(("Code Generator visited base BinaryASTNode class?!", false)); }

// Integer and Identifier Leaf Nodes. A throwaway Quadruple is returned that effectively passes back their value or name
void CodeGenerator::Visit(IntegerNode& n)    { Return({ std::nullopt, std::nullopt, std::nullopt, Operand{CmdType::NONE, std::to_string(n.value), std::to_string(n.value)} }); }
void CodeGenerator::Visit(IdentifierNode& n) { Return({ std::nullopt, std::nullopt,  std::nullopt, Operand{CmdType::NONE, n.token.str, "DWORD [ebp" + n.offset + "]"} }); }

void CodeGenerator::Visit(UnaryOperationNode& n)
{
    instructions.push_back({ Command{n.op.str, CmdType::UNARY }, fetch_instr(n.expr.get()).dest, std::nullopt, Temporary::NewTemporary() });
    Return(instructions.back());
}

void CodeGenerator::ProcessBinOp(const BinaryASTNode& n, CmdType type)
{
#ifdef OPTIMIZE_TEMPS
    const auto src1 = fetch_instr(n.left.get()).dest;
    const auto dest = Temporary::NewTemporary();
    const auto src2 = fetch_instr(n.right.get()).dest;
    instructions.push_back({ Command{n.op.str, type }, src1, src2, dest });
#else
    instructions.push_back({ Command{n.op.first, type }, fetch_instr(n.left.get()).dest, fetch_instr(n.right.get()).dest, Temporary::NewTemporary() });
#endif // OPTIMIZE_TEMPS
    Return(instructions.back());
}

void CodeGenerator::Visit(BinaryOperationNode& n) { ProcessBinOp(n, CmdType::ARITHM); }
void CodeGenerator::Visit(ConditionNode& n)       { ProcessBinOp(n, n.op.str == "&&" || n.op.str == "||" ? CmdType::LOG : CmdType::RELAT); }

void CodeGenerator::Visit(IfNode& n)
{
    const auto falseLabel = Label::NewLabel(); // if condition(s) is false this jump label is the next elseif condition start or end of if-elseif-else
    instructions.push_back({ Command{"IfFalse", CmdType::IF }, fetch_instr(n.condition.get()).dest, std::nullopt, falseLabel });
    if (n.body)
    {
        PlainVisit(n.body.get()); // Processed the body of the if or else-if, we skip the rest (via goto) and go to the end of all the chained if-elseif-else
        instructions.push_back({ Command{"Goto", CmdType::GOTO }, std::nullopt, std::nullopt,  Operand{ CmdType::LABEL, n.parentEndLabel, n.parentEndLabel} });
    }
    // The label signifying the end of this if and potentially the start of another elseif or else
    instructions.push_back({ Command{"Label", CmdType::LABEL }, std::nullopt, std::nullopt, falseLabel });
}

void CodeGenerator::Visit(IfStatementNode& n)
{
    // Label for the end of all the if-elseif-else contained
    const auto endIfLabel = Label::NewLabel();
    for (const auto& ifN : n.ifNodes) 
    { 
        // Set each child if-elseif jump label (if the branch is taken) to the end of this parent if
        ifN->parentEndLabel = endIfLabel.name; 
        PlainVisit(ifN.get()); 
    }
    if (n.elseBody) PlainVisit(n.elseBody.get());  // No need to attach a goto end here, this is end of the if-else-if-else chain anyway
    instructions.push_back({ Command{"Label", CmdType::LABEL }, std::nullopt, std::nullopt, endIfLabel });
}

void CodeGenerator::Visit(IterationNode& n) { assert(("Code Generator visited base IterationNode class?!", false)); }

void CodeGenerator::Visit(WhileNode& n)
{
    const auto startLabel = Label::NewLabel();
    instructions.push_back({ Command{"Label", CmdType::LABEL }, std::nullopt, std::nullopt, startLabel });
    const auto endLabel = Label::NewLabel();
    instructions.push_back({ Command{"IfFalse", CmdType::IF }, fetch_instr(n.condition.get()).dest, std::nullopt, endLabel });
    if (n.body)
    {
        PlainVisit(n.body.get()); // Processed the body of the while, we go back to the condition
        instructions.push_back({ Command{"Goto", CmdType::GOTO }, std::nullopt, std::nullopt, startLabel });
    }
    instructions.push_back({ Command{"Label", CmdType::LABEL }, std::nullopt, std::nullopt, endLabel });
}

void CodeGenerator::Visit(DoWhileNode& n)
{
    const auto startLabel = Label::NewLabel();
    instructions.push_back({ Command{"Label", CmdType::LABEL }, std::nullopt, std::nullopt, startLabel });
    if (n.body)
    {
        PlainVisit(n.body.get());
        instructions.push_back({ Command{"If", CmdType::IF }, fetch_instr(n.condition.get()).dest, std::nullopt, startLabel });
    }
}

void CodeGenerator::Visit(CompoundStatementNode& n) { for (const auto& statement : n.statements) PlainVisit(statement.get()); }
void CodeGenerator::Visit(StatementBlockNode& n)    { for (const auto& statement : n.statements) PlainVisit(statement.get()); }
void CodeGenerator::Visit(DeclareStatementNode& n)  { Return(GetValue(n.identifier.get())); }

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
    instructions.push_back({ Command{ n.op.str, CmdType::COPY }, fetch_instr(n.right.get()).dest, std::nullopt, GetValue(n.left.get()).dest });
//#endif // OPTIMIZE_TEMPS
}

void CodeGenerator::Visit(DeclareAssignNode& n)   { ProcessAssignment(n); }
void CodeGenerator::Visit(AssignStatementNode& n) { ProcessAssignment(n); }

void CodeGenerator::Visit(ReturnStatementNode& n)
{
    instructions.push_back({ Command{ "Return", CmdType::RET }, std::nullopt, std::nullopt, GetValue(n.expr.get()).dest });
}

void CodeGenerator::Visit(EmptyStatementNode& n) {}

/* TODO:
    -Mul and Div require special registers
    -Mov mem, mem is not possible?
    -&& and || proccessing (ie multiple conditions in one statement condition)
    -Fix/Add Nodes into the ast to accomodate main/entry point - potentially more? 
*/