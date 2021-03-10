#include "ASTVisualizer.h"
#include "AbstractSyntaxTree.h"

void ASTVisualizer::RenderAST(ASTNode& n)
{
    open_action = -1;
    if (ImGui::Button("Expand AST"))   open_action = 1; ImGui::SameLine();
    if (ImGui::Button("Collapse AST")) open_action = 0; ImGui::SameLine();
    ImGui::Checkbox("Remove Identation", &align_label_with_current_x_position);
    ImGui::Separator();

    RenderNode([&]() { n.Accept(*this); }, (void*)(intptr_t)&n, "ROOT");
}

template<class ...Args>
void ASTVisualizer::RenderNode(std::function<void()> visitCallback, void* n, const char* fmt, Args... args)
{
    if (open_action != -1) ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode(n, fmt, args...))
    {
        if (align_label_with_current_x_position)
            ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
        visitCallback();
        if (align_label_with_current_x_position)
            ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
        ImGui::TreePop();
    }
}

void ASTVisualizer::Visit(ASTNode& n)       { assert(("ASTVisualizer visited base ASTNode class?!"      , false)); }
void ASTVisualizer::Visit(UnaryASTNode& n)  { assert(("ASTVisualizer visited base UnaryASTNode class?!" , false)); }
void ASTVisualizer::Visit(BinaryASTNode& n) { assert(("ASTVisualizer visited base BinaryASTNode class?!", false)); }

void ASTVisualizer::Visit(IntegerNode& n)
{
    ImGuiTreeNodeFlags node_flags = base_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
    ImGui::TreeNodeEx((void*)(intptr_t)&n, node_flags, "INT_LITERAL:%d", n.value);
}

void ASTVisualizer::Visit(IdentifierNode& n)
{
    ImGuiTreeNodeFlags node_flags = base_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
    ImGui::TreeNodeEx((void*)(intptr_t)&n, node_flags, "%s:%s", n.name.c_str(), magic_enum::enum_name(n.type).data());
}

void ASTVisualizer::Visit(UnaryOperationNode& n)
{
    RenderNode( [&]() { n.expr->Accept(*this); }, (void*)(intptr_t)&n, "UNARY OP '%s'", n.op.first.c_str());
}

void ASTVisualizer::Visit(BinaryOperationNode& n)
{
    RenderNode( [&]() { 
        n.left->Accept(*this); 
        n.right->Accept(*this); },
        (void*)(intptr_t)&n, "BINARY OP '%s'", n.op.first.c_str());
}

void ASTVisualizer::Visit(ConditionNode& n)
{
    RenderNode( [&]() { 
        n.left->Accept(*this); 
        n.right->Accept(*this); },
        (void*)(intptr_t)&n, "CONDITION '%s'", n.op.first.c_str());
}

void ASTVisualizer::Visit(IfNode& n)
{
    RenderNode( [&]() { 
        n.condition->Accept(*this); 
        if (n.body) 
            n.body->Accept(*this); },
        (void*)(intptr_t)&n, "%s", n.type.c_str());
}

void ASTVisualizer::Visit(IfStatementNode& n)
{
    RenderNode( [&]() { 
        for (const auto& ifN : n.ifNodes) 
            ifN->Accept(*this); 
        if (n.elseBody) 
            n.elseBody->Accept(*this); },
        (void*)(intptr_t)&n, "IF_STATEMENT");
}

void ASTVisualizer::Visit(IterationNode& n) { assert(("ASTVisualizer visited base IterationNode class?!", false)); }
void ASTVisualizer::Visit(WhileNode& n)
{
    RenderNode( [&]() { 
        n.condition->Accept(*this); 
        if (n.body)  
            n.body->Accept(*this); },
        (void*)(intptr_t)&n, "WHILE");
}

void ASTVisualizer::Visit(DoWhileNode& n)
{
    RenderNode( [&]() { 
        n.condition->Accept(*this); 
        if (n.body) 
            n.body->Accept(*this); },
        (void*)(intptr_t)&n, "DO_WHILE");
}

void ASTVisualizer::Visit(CompoundStatementNode& n)
{
    RenderNode( [&]() { 
        for (const auto& statement : n.statements) 
            statement->Accept(*this); },
        (void*)(intptr_t)&n, "COMPOUND");
}

void ASTVisualizer::Visit(StatementBlockNode& n)
{
    RenderNode( [&]() { 
        for (const auto& statement : n.statements) 
            statement->Accept(*this); },
        (void*)(intptr_t)&n, "STATEMENT_BLOCK");
}

void ASTVisualizer::Visit(DeclareStatementNode& n)
{
    RenderNode( [&]() { n.identifier->Accept(*this); }, (void*)(intptr_t)&n, "DECLARE");
}

void ASTVisualizer::Visit(DeclareAssignNode& n)
{
    RenderNode( [&]() { 
        n.left->Accept(*this); 
        n.right->Accept(*this); },
        (void*)(intptr_t)&n, "DECLARE_ASSIGN '%s'", n.op.first.c_str());
}

void ASTVisualizer::Visit(AssignStatementNode& n)
{
    RenderNode( [&]() { 
        n.left->Accept(*this); 
        n.right->Accept(*this); },
        (void*)(intptr_t)&n, "ASSIGN '%s'", n.op.first.c_str());
}

void ASTVisualizer::Visit(ReturnStatementNode& n)
{
    RenderNode( [&]() { n.expr->Accept(*this); }, (void*)(intptr_t)&n, "RETURN");
}

void ASTVisualizer::Visit(EmptyStatementNode& n) {}