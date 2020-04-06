#include "CodeGenerator.h"
#include "AbstractSyntaxTree.h"

ThreeAddressCode CodeGenerator::instructions;
int CodeGenerator::temporaries = 0;
int CodeGenerator::labels = 0;

void CodeGenerator::GenerateAssembly(ASTNode* n)
{
	// set up stack/main;
	/*std::cout << "\npush rbp\nmov rbp, rsp\n";
	n->Accept(*this);
	std::cout << "pop rbp\nret\n";*/
	std::cout << "Intermediate Language Representation:\nmain:\n";
	PlainVisit(n);
	for (const auto& instruction : instructions)   // no string comparisons pls - how about string_view or something else - merge stuff
	{
		// if there is a second operand, output in the form dest = src1 op src2
		if (instruction.src2.has_value()) 
			std::cout << "\t" << instruction.dest.value() << " = " << instruction.src1.value() << " " << instruction.op.value() << " " << instruction.src2.value() << ";\n";
		else if (instruction.op.value() == "=")       std::cout << "\t" << instruction.dest.value() << " " << instruction.op.value() << " " << instruction.src1.value() << ";\n";
		else if (instruction.op.value() == "IfZ")     std::cout << "\t" << instruction.op.value() << " " << instruction.src1.value() << " Goto " << instruction.dest.value() << ";\n";
		else if (instruction.op.value() == "Label")   std::cout << instruction.dest.value() << ":\n";
		else if (instruction.op.value() == "Return")  std::cout << "\t" << instruction.op.value() << " " << instruction.dest.value() << ";\n";
		else if (instruction.op.value() == "Goto")    std::cout << "\t" << instruction.op.value() << " " << instruction.dest.value() << ":\n";
		// Unary
		else std::cout << "\t" << instruction.dest.value() << " = " << instruction.op.value() << " " << instruction.src1.value() << ";\n";
	}
}

void CodeGenerator::Visit(ASTNode& n)        { assert(("Code Generator visited base ASTNode class?!", false)); }
void CodeGenerator::Visit(UnaryASTNode& n)   { assert(("Code Generator visited base UnaryASTNode class?!", false)); }
void CodeGenerator::Visit(BinaryASTNode& n)  { assert(("Code Generator visited base BinaryASTNode class?!", false)); }
// Integer and Identifier Leaf Nodes. A throwaway Quadruple is returned that effectively passes back their value or name
void CodeGenerator::Visit(IntegerNode& n)    { Return({ std::nullopt, std::nullopt, std::nullopt, std::to_string(n.value) }); }
void CodeGenerator::Visit(IdentifierNode& n) { Return({ std::nullopt, std::nullopt,  std::nullopt, n.name }); }

void CodeGenerator::Visit(UnaryOperationNode& n)
{
	Quadruples q = GetValue(n.expr);
	instructions.push_back({ n.op.first, q.dest, std::nullopt, "_t" + std::to_string(temporaries) });
	Return(instructions.back());
	++temporaries;
}

void CodeGenerator::Visit(BinaryOperationNode& n)
{
	Quadruples q1 = GetValue(n.left);
	Quadruples q2 = GetValue(n.right);
	instructions.push_back({ n.op.first, q1.dest, q2.dest, "_t" + std::to_string(temporaries) });
	Return(instructions.back());
	++temporaries;
}

void CodeGenerator::Visit(ConditionNode& n)  // we will need logical expressions? what the fuck is happening here
{
	std::cout << "HERE";
	/*Quadruples q1 = GetValue(n.left);
	Quadruples q2 = GetValue(n.right);
	instructions.push_back({ n.op.first, q1.dest, q2.dest, "t" + std::to_string(temporaries) });
	std::cout << "\nCHECK " << "t" + std::to_string(temporaries) << "\n";
	Return(instructions.at(temporaries));
	++temporaries;*/
}

void CodeGenerator::Visit(IfNode& n)   // no else etc - would be nice, probably no if else - but first parser must be able to understand it
{
	Quadruples cond = GetValue(n.condition);
	std::string falseLabel = "_L" + std::to_string(labels);
	instructions.push_back({ "IfZ", cond.dest, std::nullopt, falseLabel });  // better encoding here? can be others than IfFalse(Z) based on cond operator?
	++labels;
	if (n.body) PlainVisit(n.body);
	instructions.push_back({ "Label", std::nullopt, std::nullopt, falseLabel });
}

void CodeGenerator::Visit(WhileNode& n)
{
	std::string startLabel = "_L" + std::to_string(labels);
	++labels;
	instructions.push_back({ "Label", std::nullopt, std::nullopt, startLabel });
	Quadruples cond = GetValue(n.condition);
	std::string endLabel = "_L" + std::to_string(labels);
	++labels;
	instructions.push_back({ "IfZ", cond.dest, std::nullopt, endLabel });
	if (n.body) PlainVisit(n.body);
	instructions.push_back({ "Goto", std::nullopt, std::nullopt, startLabel });
	instructions.push_back({ "Label", std::nullopt, std::nullopt, endLabel });
}

void CodeGenerator::Visit(CompoundStatementNode& n)
{
	for (const auto& statement : n.statements) PlainVisit(statement);
	// the way it is set up here, generation will start here, maybe have a ProgramEntryNode?  also label main:
	// for setting up stack etc? for exit as well - wont work atm - hack atm is generate
}

void CodeGenerator::Visit(DeclareStatementNode& n)
{
	// no visiting probs
	// just visit? or do nothing? Semantic Analyzer has taken care of variables being declared etc
	// it will need to do type checking also (in the future) and tell code generator size of var?
}

void CodeGenerator::Visit(AssignStatementNode& n)
{
	Quadruples q1 = GetValue(n.left);
	Quadruples q2 = GetValue(n.right);
	instructions.push_back({ n.op.first, q2.dest, std::nullopt, q1.dest });
}

void CodeGenerator::Visit(ReturnStatementNode& n)
{
	Quadruples expr = GetValue(n.expr);
	instructions.push_back({ "Return", std::nullopt, std::nullopt, expr.dest });
	// we are done here we must jump to the end or something
}

void CodeGenerator::Visit(EmptyStatementNode& n) {}

// TODO:
/*
	-Fix/Add Nodes into the ast to accomodate logical operations, main/entry point - potentially more? 
	-Start filling out the functions - output TAC into the console for start (expressions done - googo ifs!)
	-Rename+cpp+h to CodeGeneratorIR?

	switch in all couts from "\n" to '\n'
	check asserts if work
	conserve temporaries - the t0 = a * b --> c = t0 is annoying!!!
	merge some Quadruples expr = GetValue(n.expr) etc that are used only for one thing?
	need beginfunc and endfunc, jump for return to the end and allocating space at begin func?

	-CHECK STD::VISITOR-VARIANT - nah
	https://web.stanford.edu/class/archive/cs/cs143/cs143.1128/handouts/240%20TAC%20Examples.pdf page 7-8 for reduced temporaries
*/