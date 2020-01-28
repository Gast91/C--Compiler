#pragma once
#include "Lexer.h"
#include "AbstractSyntaxTree.h"

class Parser
{
private:
	Lexer* lexer;
	ASTNode* root;

	ASTNode* ParseFactor();
	ASTNode* ParseTerm();
	ASTNode* ParseExpr();   // assignment expression here? atm its a new one
	ASTNode* ParseCond();
	ASTNode* ParseIf();
	ASTNode* ParseWhile();
	ASTNode* ParseProgram();                      // main and functions and assignments/declarations - just main now - entry point
	ASTNode* ParseCompoundStatement();
	std::vector<ASTNode*> ParseStatementList();   // simplify vector here?
	ASTNode* ParseStatement();
	ASTNode* ParseAssignStatement();
	// variable ? for type?
	ASTNode* ParseEmpty();

	bool failState = false; 
	// something to count nodes out of curiosity?:P
public:
	Parser(Lexer* lex);
	~Parser();
};