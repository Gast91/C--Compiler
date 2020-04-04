#pragma once
#include <string>
#include <map>
#include <vector>
#include <iostream>

#include "Visitor.h"
#include "Error.h"

class Symbol
{
public:
	std::string name;
	// The Symbol of an identifier - not all identifiers have a symbol
	// BuiltIn Type Definitions and Nested Scopes in a symbol table dont have a type, just a name
	Symbol* type;             
	friend class SymbolTable;

	Symbol(std::string n, Symbol* t = nullptr);
	virtual ~Symbol();  // SymbolTable class will handle the deleting of Symbols

	virtual void Print() = 0;
};

// Symbols built into the language - Integers, Floats, Chars etc.
// Type is indicated by name, used as a lookup
class BuiltInSymbol : public Symbol
{
public:
	BuiltInSymbol(std::string n);

	virtual void Print() override;
};

// Actual Variable Symbols containing a variable name and a type 
// (of type Symbol(BuiltInSymbol for now, later can be user defined))
class VariableSymbol : public Symbol
{
public:
	VariableSymbol(std::string n, Symbol* t);

	virtual void Print() override;
};

// Symbols indicating a nested scope within the current scope
class NestedScope : public Symbol
{
public:
	NestedScope(std::string n);

	virtual void Print() override;
};

// Abstract Data Type for tracking various symbols in the source code
// Each SymbolTable represents a scope in the source code
class SymbolTable
{
private:
	std::map<std::string, Symbol*> symbols;
	std::string scopeName;
	int scopeLevel;
	SymbolTable* parentScope;
	friend class SemanticAnalyzer;
public:
	SymbolTable(const std::string& name, const int level, SymbolTable* parent = nullptr);
	~SymbolTable();  // Deleting of Symbols will be handled here

	bool DefineSymbol(Symbol* s);
	Symbol* LookUpSymbol(std::string& symName);

	void Print();
};

class SemanticAnalyzer : public ASTNodeVisitor
{
private:
	std::vector<SymbolTable*> symbolTable;
	SymbolTable* currentScope;

	bool failState = false;
public:
	SemanticAnalyzer();
	~SemanticAnalyzer();

	// Inherited via ASTNodeVisitor
	virtual void Visit(ASTNode& n)               override;
	virtual void Visit(UnaryASTNode& n)          override;
	virtual void Visit(BinaryASTNode& n)         override;
	virtual void Visit(IntegerNode& n)           override;
	virtual void Visit(IdentifierNode& n)        override;
	virtual void Visit(UnaryOperationNode& n)    override;
	virtual void Visit(BinaryOperationNode& n)   override;
	virtual void Visit(ConditionNode& n)         override;
	virtual void Visit(IfNode& n)                override;
	virtual void Visit(WhileNode& n)             override;
	virtual void Visit(CompoundStatementNode& n) override;
	virtual void Visit(DeclareStatementNode& n)  override;
	virtual void Visit(AssignStatementNode& n)   override;
	virtual void Visit(ReturnStatementNode& n)   override;
	virtual void Visit(EmptyStatementNode& n)    override;

	void Print() const;
	bool Success() const;
};