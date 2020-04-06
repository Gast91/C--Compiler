#pragma once
#include <sstream>
#include <vector>
#include <fstream>

class ASTNode;

std::string GenerateID(const ASTNode* node, const char* ID);
std::string GenerateJSONHeader(std::ofstream& out, const ASTNode* root, const char* rootID, std::vector<std::string>& config);
void GenerateJSONFooter(std::ofstream& out, const std::vector<std::string>& config);
std::string GenerateJSON(std::ofstream& out, const ASTNode* node, const char* ID, std::string& parentID, std::string name, std::vector<std::string>& config);