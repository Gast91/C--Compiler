#include "Symbol.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------Symbol Definitions------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------
Symbol::Symbol(std::string n, std::string off, Symbol * t) : name(n), offset(off), type(t) {}

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------BuiltInSymbol Definitions-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------
BuiltInSymbol::BuiltInSymbol(std::string n) : Symbol(n) {}
void BuiltInSymbol::Print() { std::cout << "<" << name << ">\n";  }

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------VariableSymbol Definitions----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------
VariableSymbol::VariableSymbol(std::string n, std::string off, Symbol* t) : Symbol(n, off, t) {}
void VariableSymbol::Print() 
{
    std::cout << name << ": ";
    type->Print();
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------NestedScope Definitions-------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------
NestedScope::NestedScope(std::string n) : Symbol(n) {}
void NestedScope::Print() { std::cout << name << " <NESTED_SCOPE>\n"; }

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------SymbolTable Definitions-------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------
SymbolTable::SymbolTable(const std::string& name, const int level, SymbolTable* parent) : scopeName(name), scopeLevel(level), parentScope(parent)
{ 
    // Built-In Symbols will be redefined for every new scope
    DefineSymbol(new BuiltInSymbol("int")); 
    // "New" Built In Symbols will have to be declared added here, in order to be used
}

SymbolTable::~SymbolTable() { for (const auto& s : symbols) delete s.second; }

bool SymbolTable::DefineSymbol(Symbol* s)
{
    // Redefinition of an identifier can happen in a nested scope. The nested identifier
    // will hide the one in the parent scope. But redefinition cannot happen in the same scope.
    const auto[it, success] = symbols.insert({ s->name, s });
    return success;
}

Symbol* SymbolTable::LookUpSymbol(const std::string& symName)
{
    // If the identifier is not found in the current scope, this function will
    // recursively check the parent scope all the way up to the global scope.
    // If it at the end the identifier is not present anywhere, there is a semantic error.
    if (const auto it = symbols.find(symName); it != symbols.end()) return it->second;
    else return parentScope ? parentScope->LookUpSymbol(symName) : nullptr;
}

void SymbolTable::Print()
{
    std::cout << "Declared Symbols in '" << scopeName << "' <Lvl: " << std::to_string(scopeLevel) << "> :\n";
    for (const auto& s : symbols) s.second->Print();
    std::cout << '\n';
}

/*
Add:	-Each VariableSymbol should have an address associated with it
         So that the code generator later on - or assembly generator will know its stack offset
        -Rename VariableSymbol to IdentifierSymbol?
        -Rename Symbol.h,Symbol.cpp to SymbolTable? <----
        -Missing Type Checking
        -For more types->redefinition to happen must be same name and type
         main can be a variable but you cant have two identifiers with type function
*/