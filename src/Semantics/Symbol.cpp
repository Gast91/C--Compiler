#include <imgui.h>

#include "Symbol.h"

template<typename ...Args>
[[maybe_unused]] bool RenderNodeColumns(const char* nodeLabel, const ImGuiTreeNodeFlags nodeFlags, Args... columnText)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    const bool nodeVisible = ImGui::TreeNodeEx(nodeLabel, nodeFlags);
    auto nextColumn = [](auto arg) {
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(arg);
    };
    (nextColumn(columnText), ...);

    return nodeVisible;
}
static const ImGuiTreeNodeFlags leafFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth;

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------Symbol Definitions------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------
Symbol::Symbol(std::string n, std::string off, Symbol * t) : name(n), offset(off), type(t) {}

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------BuiltInSymbol Definitions-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------
BuiltInSymbol::BuiltInSymbol(std::string n) : Symbol(n) {}

void BuiltInSymbol::Render() const { RenderNodeColumns(name.c_str(), leafFlags, "Built-In Symbol", "--"); }

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------VariableSymbol Definitions----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------
VariableSymbol::VariableSymbol(std::string n, std::string off, Symbol* t) : Symbol(n, off, t) {}

void VariableSymbol::Render() const { RenderNodeColumns(name.c_str(), leafFlags, type->name.c_str(), "--"); }

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------NestedScope Definitions-------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------
NestedScope::NestedScope(std::string n) : Symbol(n) {}

void NestedScope::Render() const { RenderNodeColumns(name.c_str(), leafFlags, "Nested Scope", "--"); }

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

// If the identifier is not found in the current scope, this function will
// recursively check the parent scope all the way up to the global scope.
// If it at the end the identifier is not present anywhere, there is a semantic error.
Symbol* SymbolTable::LookUpSymbol(const std::string& symName)
{ 
    if (const auto it = symbols.find(symName); it != symbols.end()) return it->second;
    else return parentScope ? parentScope->LookUpSymbol(symName) : nullptr;
}

void SymbolTable::Render() const
{
    if (RenderNodeColumns(scopeName.c_str(), ImGuiTreeNodeFlags_SpanFullWidth, "Nested Scope", std::to_string(scopeLevel).c_str()))
    {
        for (const auto& s : symbols) s.second->Render();
        ImGui::TreePop();
    }
}

/* TODO:
    -Missing Type Checking
    -For more types->redefinition to happen must be same name and type
     main can be a variable but you cant have two identifiers with type function
*/