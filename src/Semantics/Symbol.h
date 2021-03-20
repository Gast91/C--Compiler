#pragma once
#include <string>
#include <memory>
#include <map>
#include <vector>

class Symbol
{
protected:
    std::string name;
    std::string offset;
    // The Symbol of an identifier - not all identifiers have a symbol
    // BuiltIn Type Definitions and Nested Scopes in a symbol table dont have a type, just a name
    const Symbol* type;   
    friend class SymbolTable;
    friend class SemanticAnalyzer;
public:
    Symbol(const std::string& n, const std::string& off = "0", const Symbol* t = nullptr) : name(n), offset(off), type(t) {}
    virtual ~Symbol() = default;

    std::string GetName() const { return name; }
    virtual void Render() const = 0;
};

// Symbols built into the language - Integers, Floats, Chars etc.
// Type is indicated by name, used as a lookup
class BuiltInSymbol : public Symbol
{
public:
    BuiltInSymbol(const std::string& n) : Symbol(n) {}
    virtual ~BuiltInSymbol() = default;

    virtual void Render() const override;
};

// Actual Variable Symbols containing a variable name and a type 
// (of type Symbol(BuiltInSymbol for now, later can be user defined))
class VariableSymbol : public Symbol
{
public:
    VariableSymbol(const std::string& n, const std::string& off, const Symbol* t) : Symbol(n, off, t) {}
    virtual ~VariableSymbol() = default;

    virtual void Render() const override;
};

// Symbols indicating a nested scope within the current scope
class NestedScope : public Symbol
{
public:
    NestedScope(const std::string& n) : Symbol(n) {}
    virtual ~NestedScope() = default;

    virtual void Render() const override;
};

// class FunctionSymbol : public Symbol ...
// class UserDefinedSymbol : public Symbol ...

// Abstract Data Type for tracking various symbols in the source code
// Each SymbolTable represents a scope in the source code
class SymbolTable
{
private:
    std::map<std::string, std::unique_ptr<Symbol>> symbols;
    const std::string scopeName;
    const int scopeLevel;
    SymbolTable* parentScope;
    friend class SemanticAnalyzer;
public:
    SymbolTable(const std::string& name, const int level, SymbolTable* parent = nullptr);

    [[maybe_unused]] bool DefineSymbol(std::unique_ptr<Symbol> s);
    const Symbol* LookUpSymbol(const std::string& symName) const;

    void Render(int isOpen) const;
};