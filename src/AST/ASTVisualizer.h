#pragma once
#include <imgui.h>
#include <functional>

#include "Visitor.h"
#include "../Util/ModuleManager.h"

class ASTVisualizer : public ASTNodeVisitor, public IObserver<ASTNode>
{
private:
    ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    bool align_label_with_current_x_position = false;
    int open_action = -1;

    ASTNode* root = nullptr;

    std::function<void()> renderExtras;

    struct ImRect
    {
        ImVec2 min;
        ImVec2 max;
    } nodeRect;

    const ImColor TreeLineColor = ImColor(128, 128, 128, 255);
    const float SmallOffsetX = -11.0f;
    const float HorizontalTreeLineSize = 8.0f;

    template<class ...Args>
    ImRect RenderNode(std::function<void()> visitCallback, void* n, const char* fmt, Args...);
public:
    void SetExtrasToRender(std::function<void()> extras) { renderExtras = extras; }
    void RenderAST();

    // Inherited via ASTNodeVisitor
    void Visit(ASTNode& n)               override;
    void Visit(UnaryASTNode& n)          override;
    void Visit(BinaryASTNode& n)         override;
    void Visit(IntegerNode& n)           override;
    void Visit(IdentifierNode& n)        override;
    void Visit(UnaryOperationNode& n)    override;
    void Visit(BinaryOperationNode& n)   override;
    void Visit(ConditionNode& n)         override;
    void Visit(IfNode& n)                override;
    void Visit(IfStatementNode& n)       override;
    void Visit(IterationNode& n)         override;
    void Visit(WhileNode& n)             override;
    void Visit(DoWhileNode& n)           override;
    void Visit(StatementBlockNode& n)    override;
    void Visit(CompoundStatementNode& n) override;
    void Visit(DeclareStatementNode& n)  override;
    void Visit(DeclareAssignNode& n)     override;
    void Visit(AssignStatementNode& n)   override;
    void Visit(ReturnStatementNode& n)   override;
    void Visit(EmptyStatementNode& n)    override;

    // Inherited via IObserver
    virtual void Update(ASTNode* n) override { root = n; }
};