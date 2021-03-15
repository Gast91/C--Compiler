#include "Symbol.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------Symbol Definitions------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------
Symbol::Symbol(std::string n, std::string off, Symbol * t) : name(n), offset(off), type(t) {}

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------BuiltInSymbol Definitions-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------
BuiltInSymbol::BuiltInSymbol(std::string n) : Symbol(n) {}
void BuiltInSymbol::Print()  const { std::cout << "<" << name << ">\n";  }
void BuiltInSymbol::Render() const
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    // SymbolName - change name to pointer to this and name is just an if or whatever rather than generate???
    ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);
    ImGui::TableNextColumn();   // SymbolType
    ImGui::TextUnformatted("Built-In Symbol");
    ImGui::TableNextColumn();   // Nested Level
    ImGui::TextDisabled("--");
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------VariableSymbol Definitions----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------
VariableSymbol::VariableSymbol(std::string n, std::string off, Symbol* t) : Symbol(n, off, t) {}
void VariableSymbol::Print() const
{
    std::cout << name << ": ";
    type->Print();
}
void VariableSymbol::Render() const
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    // SymbolName - the node itself | Type | NestedLvl?
    // 
    // SymbolName - change name to pointer to this and name is just an if or whatever rather than generate???
    ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);
    ImGui::TableNextColumn();   // SymbolType
    ImGui::TextUnformatted(type->name.c_str());
    ImGui::TableNextColumn();   // Nested Level
    ImGui::TextDisabled("--");
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------NestedScope Definitions-------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------
NestedScope::NestedScope(std::string n) : Symbol(n) {}
void NestedScope::Print()  const { std::cout << name << " <NESTED_SCOPE>\n"; }
void NestedScope::Render() const
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    // SymbolName - change name to pointer to this and name is just an if or whatever rather than generate???
    ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);
    ImGui::TableNextColumn();   // SymbolType
    ImGui::TextUnformatted("Nested Scope");    // ???
    ImGui::TableNextColumn();   // Nested Level
    ImGui::TextDisabled("--");  // Level here?
}

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

void SymbolTable::Print() const
{
    std::cout << "Declared Symbols in '" << scopeName << "' <Lvl: " << std::to_string(scopeLevel) << "> :\n";
    for (const auto& s : symbols) s.second->Print();
    std::cout << '\n';
}
void SymbolTable::Render() const
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    // SymbolName - change name to pointer to this and name is just an if or whatever rather than generate???
    if (ImGui::TreeNodeEx(scopeName.c_str(), ImGuiTreeNodeFlags_SpanFullWidth))
    {
        ImGui::TableNextColumn();   // SymbolType
        ImGui::TextUnformatted("Nested Scope");   // ????
        ImGui::TableNextColumn();   // Nested Level
        ImGui::TextDisabled(std::to_string(scopeLevel).c_str());  // Level here?
        for (const auto & s : symbols) s.second->Render();
        ImGui::TreePop();
    }   
}


/* TODO:
    -Missing Type Checking
    -For more types->redefinition to happen must be same name and type
     main can be a variable but you cant have two identifiers with type function
*/