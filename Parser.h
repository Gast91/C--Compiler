#pragma once
#include "Lexer.h"
#include "AbstractSyntaxTree.h"

class Parser
{
private:
	Lexer* lexer;
	ASTNode* root;

	// functions that determing factors,terms and expressions
	ASTNode* ParseFactor();
	ASTNode* ParseTerm();
	ASTNode* ParseExpr();   // assignment expression here? or new one?
	ASTNode* ParseCond();
	ASTNode* ParseIf();
	// TODO:
	ASTNode* ParseProgram();  // main and functions? or compound statements
	ASTNode* ParseCompoundStatement();
	std::vector<ASTNode*>& ParseStatementList();  // list
	ASTNode* ParseStatement();
	ASTNode* ParseAssignment();
	// variable ?
	// assignment?
	ASTNode* ParseEmpty();

	bool failState = false;
public:
	Parser(Lexer* lex);
	~Parser();
};