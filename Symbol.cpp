#include "Symbol.h"
#include "AbstractSyntaxTree.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------Symbol Definitions------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------
Symbol::Symbol(std::string n, Symbol * t) : name(n), type(t) {}

Symbol::~Symbol() {}    // SymbolTable class will handle the deleting of Symbols

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------BuiltInSymbol Definitions-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------
BuiltInSymbol::BuiltInSymbol(std::string n) : Symbol(n) {}

void BuiltInSymbol::Print() { std::cout << "<" << name << ">\n";  }

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------VariableSymbol Definitions----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------
VariableSymbol::VariableSymbol(std::string n, Symbol * t) : Symbol(n, t) {}

void VariableSymbol::Print() 
{
	std::cout << name << ": ";
	type->Print();
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------NestedScope Definitions-------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------
NestedScope::NestedScope(std::string n) : Symbol(n) {}

void NestedScope::Print() { std::cout << name << " <Nested Scope>\n"; }

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------SymbolTable Definitions-------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------
SymbolTable::SymbolTable(const std::string& name, const int level, SymbolTable* parent) : scopeName(name), scopeLevel(level), parentScope(parent)
{ 
	// Built-In Symbols will be redefined for every new scope
	DefineSymbol(new BuiltInSymbol("INTEGER")); 
	// "New" Built In Symbols will have to be declared added here, in order to be used
}

SymbolTable::~SymbolTable() { for (const auto& s : symbols) delete s.second; }

bool SymbolTable::DefineSymbol(Symbol* s)
{
	// Redefinition of an identifier can happen in a nested scope. The nested identifier will hide the one in the parent.
	// But redefinition cannot happen in the same scope.
	if (symbols.find(s->name) == symbols.end())
	{
		symbols[s->name] = s;
		return true;
	}
	return false;
}

Symbol* SymbolTable::LookUpSymbol(std::string& symName)
{
	// If the identifier is not found in the current scope, this function will
	// recursively check the parent scope all the way up to the global scope.
	// If it at the end the identifier is not present anywhere, there is a semantic error.
	try { return symbols.at(symName); }
	catch (std::out_of_range ex)  { return parentScope ? parentScope->LookUpSymbol(symName) : nullptr; }
}

void SymbolTable::Print()
{
	std::cout << "Declared Symbols in '" << scopeName << "' <Lvl: " << std::to_string(scopeLevel) << "> :\n";
	for (const auto& s : symbols) s.second->Print();
	std::cout << "\n";
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------SemanticAnalyzer Definitions------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------
SemanticAnalyzer::SemanticAnalyzer() 
{ 
	// At the start, the only Scope/Symbol Table is the Global one (which also has no parent)
	symbolTable.push_back(new SymbolTable("GLOBAL_SCOPE", 1));
	currentScope = symbolTable.back();
}

SemanticAnalyzer::~SemanticAnalyzer() { for (const auto& scope : symbolTable) delete scope; }

void SemanticAnalyzer::Visit(ASTNode& n)
{
	// If this gets called it will be a stack overflow/Infinite loop and things must have gotten catastrophically wrong
	n.Accept(*this);
}

void SemanticAnalyzer::Visit(UnaryASTNode& n)
{
	n.expr->Accept(*this);
}

void SemanticAnalyzer::Visit(BinaryASTNode& n)
{
	n.left->Accept(*this);
	n.right->Accept(*this);
}

void SemanticAnalyzer::Visit(IntegerNode& n) {}

void SemanticAnalyzer::Visit(IdentifierNode& n)
{
	if (!currentScope->LookUpSymbol(n.value))
	{
		failState = true;
		throw SymbolNotFoundException("\nUse of undeclared identifier '" + n.value + "' in scope '"
										+ currentScope->scopeName + "'<Lvl: " + std::to_string(currentScope->scopeLevel) + ">\n");
	}
}

void SemanticAnalyzer::Visit(BinaryOperationNode& n)
{
	n.left->Accept(*this);
	n.right->Accept(*this);
}

void SemanticAnalyzer::Visit(ConditionNode& n)
{
	n.left->Accept(*this);
	n.right->Accept(*this);
}

void SemanticAnalyzer::Visit(IfNode& n)
{
	// Visit Condition, Identifiers in the condition belong to the current scope
	n.condition->Accept(*this);

	// Semantic Analysis up unti the condition has been performed, body belongs to a different, nested scope
	// Generate a name for it and add the nested scope as a symbol into the parent scope (current)
	std::string nestedScopeName = GenerateID(&n, "IF_");
	currentScope->DefineSymbol(new NestedScope(nestedScopeName));

	// New nested scope with the nested scope name, at a greater depth than the current with the current scope as its parent
	SymbolTable* nestedScope = new SymbolTable(nestedScopeName, currentScope->scopeLevel + 1, currentScope);

	// Current scope becomes this new scope
	symbolTable.push_back(nestedScope);
	currentScope = nestedScope;

	// Perform Semantic Analysis to the "contents" of this new scope
	n.body->Accept(*this);

	// After we are done with the body of this nested statement we go back to the parent scope
	currentScope = nestedScope->parentScope;
}

void SemanticAnalyzer::Visit(WhileNode& n)
{
	// Visit Condition, Identifiers in the condition belong to the current scope
	n.condition->Accept(*this);

	// Semantic Analysis up unti the condition has been performed, body belongs to a different, nested scope
	// Generate a name for it and add the nested scope as a symbol into the parent scope (current)
	std::string nestedScopeName = GenerateID(&n, "WHILE_");
	currentScope->DefineSymbol(new NestedScope(nestedScopeName));

	// New nested scope with the nested scope name, at a greater depth than the current with the current scope as its parent
	SymbolTable* nestedScope = new SymbolTable(nestedScopeName, currentScope->scopeLevel + 1, currentScope);

	// Current scope becomes this new scope
	symbolTable.push_back(nestedScope);
	currentScope = nestedScope;

	// Perform Semantic Analysis to the "contents" of this new scope
	n.body->Accept(*this);

	// After we are done with the body of this nested statement we go back to the parent scope
	currentScope = nestedScope->parentScope;
}

void SemanticAnalyzer::Visit(CompoundStatementNode& n)
{
	for (const auto& statement : n.statements) statement->Accept(*this);
}

void SemanticAnalyzer::Visit(DeclareStatementNode& n)
{
	// Get Declaration Node's type (int,float,etc)						- Hard-Coded atm
	std::string typeName = "INTEGER";
	// Look it up in the symbol table and get the symbolType object
	Symbol* symbolType = currentScope->LookUpSymbol(typeName);
	// Get the variable name from the Declaration's Identifier Node
	std::string variableName = n.identifier->value;						// name would be better than value
	// Define a new VarSymbol using variable name and symbolType
	Symbol* variableSymbol = new VariableSymbol(variableName, symbolType);
	if (!currentScope->DefineSymbol(variableSymbol))
	{
		failState = true;
		// The redefined identifier will not be stored as the program is semantically wrong and Semantic Analysis will stop
		delete variableSymbol; // Clean up this temporary and throw the RedefinitionException
		throw SymbolRedefinitionException("Redefinition of identifier '" + variableName + "' in scope '"
			+ currentScope->scopeName + "'<Lvl: " + std::to_string(currentScope->scopeLevel) + ">\n");
	}
}

void SemanticAnalyzer::Visit(AssignStatementNode& n)
{
	std::string symbolName = static_cast<IdentifierNode*>(n.left)->value;
	if (!currentScope->LookUpSymbol(symbolName))
	{
		failState = true;
		throw SymbolNotFoundException("\nUse of undeclared identifier '" + symbolName + "' in scope '"
										+ currentScope->scopeName + "'<Lvl: " + std::to_string(currentScope->scopeLevel) + ">\n");
	}
	// left's value has been extracted and if we reached here we know the symbol is in the table so no need to visit left
	n.right->Accept(*this);  
}

void SemanticAnalyzer::Visit(EmptyStatementNode& n) {}

void SemanticAnalyzer::Print() const
{ 
	std::cout << (failState ? "\nSemantic Analysis FAILED " : "\nSemantic Analysis Complete ") << "->Dumping Scope / Symbol Information : \n\n";
	for (const auto& scope : symbolTable) scope->Print();
}

bool SemanticAnalyzer::Success() const { return !failState; }

/*
Add:	-Each VariableSymbol should have an address associated with it
		 So that the code generator later on - or assembly generator will know its stack offset
		-Rename VariableSymbol to IdentifierSymbol?
*/