#include "Utility.h"

std::string GenerateID(const ASTNode* node, const char* ID)
{
	std::stringstream ss;
	ss << static_cast<const void*>(node);
	std::string id = ss.str();
	id.insert(0, ID);
	return id;
}

std::string GenerateJSONHeader(std::ofstream& out, const ASTNode* root, const char* rootID, std::vector<std::string>& config)
{
	out << "config = {\n\tcontainer: \"#AST\"\n};\n\n";
	config.push_back("config");
	const std::string id = GenerateID(root, rootID);
	out << id << " = {\n\ttext: { name: \"ROOT\" }\n};\n\n";
	config.push_back(id);
	return id;
}

void GenerateJSONFooter(std::ofstream& out, const std::vector<std::string>& config) 
{
	out << "simple_chart_config = [\n    ";
	for (unsigned int i = 0; i < config.size(); ++i) { if (i != 0) out << ", "; out << config.at(i); }
	out << "\n];";
}

std::string GenerateJSON(std::ofstream& out, const ASTNode* node, const char* ID, std::string& parentID, std::string name, std::vector<std::string>& config)
{
	const std::string nodeID = GenerateID(node, ID);
	out << nodeID << " = {\n\tparent: " << parentID <<
		",\n\ttext: { name: \"" << name << "\" }\n};\n\n";
	config.push_back(nodeID);
	return nodeID;
}