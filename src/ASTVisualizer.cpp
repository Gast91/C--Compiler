#include "ASTVisualizer.h"
#include "AbstractSyntaxTree.h"

void ASTVisualizer::PrintAST(ASTNode& n)
{
    // IF ROOT EXISTS!!!!!! then setNextopen is safe
    ImGui::Begin("Parser Output");
    if (ImGui::Button("Expand AST"));//   open_action = 1;
    ImGui::SameLine();
    if (ImGui::Button("Collapse AST"));// open_action = 0;
    ImGui::Separator();
    //if (open_action != -1) ImGui::SetNextItemOpen(open_action != 0);

    if (ImGui::TreeNode((void*)(intptr_t)&n, "ROOT"))
    {
        if (align_label_with_current_x_position)
            ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
        n.Accept(*this);
        if (align_label_with_current_x_position)
            ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
        ImGui::TreePop();
    }
    ImGui::End();
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
    //if (open_action != -1) ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode((void*)(intptr_t)&n, "UNARY OP '%s'", n.op.first.c_str()))
    {
        if (align_label_with_current_x_position)
            ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
        n.expr->Accept(*this);
        if (align_label_with_current_x_position)
            ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
        ImGui::TreePop();
    }
}

void ASTVisualizer::Visit(BinaryOperationNode& n)
{
    //if (open_action != -1) ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode((void*)(intptr_t)&n, "BINARY OP '%s'", n.op.first.c_str()))
    {
        if (align_label_with_current_x_position)
            ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
        n.left->Accept(*this);
        n.right->Accept(*this);
        if (align_label_with_current_x_position)
            ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
        ImGui::TreePop();
    }
}

void ASTVisualizer::Visit(ConditionNode& n)
{
    //if (open_action != -1) ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode((void*)(intptr_t)&n, "CONDITION '%s'", n.op.first.c_str()))
    {
        if (align_label_with_current_x_position)
            ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
        n.left->Accept(*this);
        n.right->Accept(*this);
        if (align_label_with_current_x_position)
            ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
        ImGui::TreePop();
    }
}

void ASTVisualizer::Visit(IfNode& n)
{
    //if (open_action != -1) ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode((void*)(intptr_t)&n, "%s", n.type.c_str()))
    {
        if (align_label_with_current_x_position)
            ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
        n.condition->Accept(*this);
        if (n.body) n.body->Accept(*this);
        if (align_label_with_current_x_position)
            ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
        ImGui::TreePop();
    }
}

void ASTVisualizer::Visit(IfStatementNode& n)
{
    //if (open_action != -1) ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode((void*)(intptr_t)&n, "IF_STATEMENT"))
    {
        if (align_label_with_current_x_position)
            ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
        for (const auto& ifN : n.ifNodes) ifN->Accept(*this);
        if (n.elseBody) n.elseBody->Accept(*this);
        if (align_label_with_current_x_position)
            ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
        ImGui::TreePop();
    }
}

void ASTVisualizer::Visit(IterationNode& n) { assert(("ASTVisualizer visited base IterationNode class?!", false)); }
void ASTVisualizer::Visit(WhileNode& n)
{
    //if (open_action != -1) ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode((void*)(intptr_t)&n, "WHILE"))
    {
        if (align_label_with_current_x_position)
            ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
        n.condition->Accept(*this);
        if (n.body) n.body->Accept(*this);
        if (align_label_with_current_x_position)
            ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
        ImGui::TreePop();
    }
}

void ASTVisualizer::Visit(DoWhileNode& n)
{
    //if (open_action != -1) ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode((void*)(intptr_t)&n, "DO_WHILE"))
    {
        if (align_label_with_current_x_position)
            ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
        n.condition->Accept(*this);
        if (n.body) n.body->Accept(*this);
        if (align_label_with_current_x_position)
            ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
        ImGui::TreePop();
    }
}

void ASTVisualizer::Visit(CompoundStatementNode& n)
{
    //if (open_action != -1) ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode((void*)(intptr_t)&n, "COMPOUND"))
    {
        if (align_label_with_current_x_position)
            ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
        for (const auto& statement : n.statements) statement->Accept(*this);
        if (align_label_with_current_x_position)
            ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
        ImGui::TreePop();
    }
}

void ASTVisualizer::Visit(StatementBlockNode& n)
{
    //if (open_action != -1) ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode((void*)(intptr_t)&n, "STATEMENT_BLOCK"))
    {
        if (align_label_with_current_x_position)
            ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
        for (const auto& statement : n.statements) statement->Accept(*this);
        if (align_label_with_current_x_position)
            ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
        ImGui::TreePop();
    }
}

void ASTVisualizer::Visit(DeclareStatementNode& n)
{
    //if (open_action != -1) ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode((void*)(intptr_t)&n, "DECLARE"))
    {
        if (align_label_with_current_x_position)
            ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
        n.identifier->Accept(*this);
        if (align_label_with_current_x_position)
            ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
        ImGui::TreePop();
    }
}

void ASTVisualizer::Visit(DeclareAssignNode& n)
{
    //if (open_action != -1) ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode((void*)(intptr_t)&n, "DECLARE_ASSIGN '%s'", n.op.first.c_str()))
    {
        if (align_label_with_current_x_position)
            ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
        n.left->Accept(*this);
        n.right->Accept(*this);
        if (align_label_with_current_x_position)
            ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
        ImGui::TreePop();
    }
}

void ASTVisualizer::Visit(AssignStatementNode& n)
{
    //if (open_action != -1) ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode((void*)(intptr_t)&n, "ASSIGN '%s'", n.op.first.c_str()))
    {
        if (align_label_with_current_x_position)
            ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
        n.left->Accept(*this);
        n.right->Accept(*this);
        if (align_label_with_current_x_position)
            ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
        ImGui::TreePop();
    }
}

void ASTVisualizer::Visit(ReturnStatementNode& n)
{
    //if (open_action != -1) ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode((void*)(intptr_t)&n, "RETURN"))
    {
        if (align_label_with_current_x_position)
            ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
            n.expr->Accept(*this);
        if (align_label_with_current_x_position)
            ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
        ImGui::TreePop();
    }
}

void ASTVisualizer::Visit(EmptyStatementNode& n) {}