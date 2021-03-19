#pragma once

class ASTNode;

struct ActionButton
{
    const char* label;
    float width;
    std::function<void()> action;
};

std::string GenerateID(const ASTNode* node, const char* ID);
void HelpMarker(const char* desc);