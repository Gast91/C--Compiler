#include "Parser.h"

// FACTOR := (ADD | SUB ) FACTOR | INTEGER | IDENTIFIER | LPAR EXPRESSION RPAR
ASTNode* Parser::ParseFactor()
{
	std::string current = lexer->GetCurrentToken();
	if (current == "+")         // just a plus before the number or identifier
	{
		lexer->Consume("+");
		return new UnaryASTNode(Token::Add, ParseFactor());
	}
	else if (current == "-")   // Negation
	{
		lexer->Consume("-");
		return new UnaryASTNode(Token::Sub, ParseFactor());
	}
	else if (lexer->IsInteger(current))
	{
		lexer->Consume(current);
		return new IntegerNode(current);
	}
	else if (lexer->IsIdentifier(current, true))
	{
		lexer->Consume(current);
		return new IdentifierNode(current);
	}
	else if (current == "(")          // some other way please
	{
		lexer->Consume("(");          // preferably make it an enum somehow
		ASTNode* node = ParseExpr();
		lexer->Consume(")");
		return node;
	}
	else throw UnexpectedTokenException("Unexpected token '" + current + "'");
}
// TERM := FACTOR ((MUL | DIV) FACTOR)*  <---
ASTNode* Parser::ParseTerm()
{
	ASTNode* node = ParseFactor();

	while (lexer->GetCurrentToken() == "*" || lexer->GetCurrentToken() == "/")
	{
		Token op;
		if (lexer->GetCurrentToken() == "*")
		{
			lexer->Consume("*");
			op = Token::Mul;
		}
		else if (lexer->GetCurrentToken() == "/")
		{
			lexer->Consume("/");
			op = Token::Div;
		}
		// Token op = lexer->GetCurrentToken();        CHANGE WHILE CONTENTS TO THIS
		// lexer->consume(lexer->GetCurrentToken());   SWAP TO vector of tokens rather than strings
		// node = new BinaryOperationNode(node, op, ParseFactor());
		node = new BinaryOperationNode(node, op, ParseFactor());
	}
	return node;
}
// EXPRESSION := TERM ((PLUS | MINUS) TERM)* <---
ASTNode* Parser::ParseExpr()
{
	ASTNode* node = ParseTerm();

	while (lexer->GetCurrentToken() == "+" || lexer->GetCurrentToken() == "-")
	{
		Token op;
		if (lexer->GetCurrentToken() == "+")
		{
			lexer->Consume("+");
			op = Token::Add;
		}
		else if (lexer->GetCurrentToken() == "-")
		{
			lexer->Consume("-");
			op = Token::Sub;
		}
		// Token op = lexer->GetCurrentToken();        CHANGE WHILE CONTENTS TO THIS
		// lexer->consume(lexer->GetCurrentToken());   SWAP TO vector of tokens rather than strings
		// node = new BinaryOperationNode(node, op, ParseTerm());
		node = new BinaryOperationNode(node, op, ParseTerm());
	}
	return node;
}
//CONDITION := EXPRESSION (LESS | MORE) EXPRESSION   [MORE NEDDED HERE]
ASTNode* Parser::ParseCond()
{
	ASTNode* node = ParseExpr();

	while (lexer->GetCurrentToken() == "<" || lexer->GetCurrentToken() == ">") // <=, >=, ==, != at least
	{
		Token op;
		if (lexer->GetCurrentToken() == "<")
		{
			lexer->Consume("<");
			op = Token::Less;
		}
		else if (lexer->GetCurrentToken() == ">")
		{
			lexer->Consume(">");
			op = Token::More;
		}
		// Token op = lexer->GetCurrentToken();        CHANGE WHILE CONTENTS TO THIS
		// lexer->consume(lexer->GetCurrentToken());   SWAP TO vector of tokens rather than strings
		// node = new ConditionNode(node, op, ParseExpr());
		node = new ConditionNode(node, op, ParseExpr());
	}
	return node;
}
// IF_STATEMENT =: IF_KEY LPAR CONDITION RPAR { COMPOUND_STATEMENT }  [MORE NEEDED HERE]
ASTNode* Parser::ParseIf()
{
	// We already know the token is an if, begin parsing the statement
	lexer->Consume("if");
	lexer->Consume("(");
	ASTNode* conditionNode = ParseCond();
	lexer->Consume(")");

	// Body of If statement can be a collection of statements
	ASTNode* ifBody = ParseCompoundStatement();

	// NOT THIS WAY, IF IS A TERNARY NODE - ATLEAST?
	//if (lexer->getCurrentToken() == "else") lexer->consume("else");
	//if (lexer->getCurrentToken() == "if")
	//{
	//	// new if node here
	//	// ifelseNode = ParseIf();  || cannot be parse if cause else if does not have else if? or is it fine cause there can be another else if after or else
	//}
	//else if(lexer->getCurrentToken() == "{")
	//{
	//	// just body here nothing else
	//	lexer->consume("}");
	//}
	// else if | else - on their own?
	// return new If(condition, body, else if vector?, elsebody, Token::If);

	return new IfNode(conditionNode, ifBody);  // if here must have slots of multiple ifelse and 1 else, how? vector?
}
// WHILE_STATEMENT := WHILE LPAR CONDITION RPAR { COMPOUND_STATEMENT }
ASTNode* Parser::ParseWhile()
{
	lexer->Consume("while");
	lexer->Consume("(");
	ASTNode* conditionNode = ParseCond();
	lexer->Consume(")");

	// Body of while statement can be a collection of statements
	ASTNode* whileBody = ParseCompoundStatement();

	return new WhileNode(conditionNode, whileBody);
}
// PROGRAM := int main LPAR RPAR { COMPOUND_STATEMENT }
ASTNode* Parser::ParseProgram()                 // hacky way for only main now
{
	lexer->Consume("int");
	lexer->Consume("main");
	lexer->Consume("(");
	lexer->Consume(")");

	ASTNode* node = ParseCompoundStatement();

	return node;
}
// COMPOUND_STATEMENT := LCUR STATEMENT_LIST RCUR
ASTNode* Parser::ParseCompoundStatement()
{
	lexer->Consume("{");
	CompoundStatementNode* compound = new CompoundStatementNode();

	for (const auto& statement : ParseStatementList()) compound->Push(statement);
	return compound;
}
// STATEMENT_LIST := STATEMENT | STATEMENT SEMICOLON STATEMENT_LIST
std::vector<ASTNode*> Parser::ParseStatementList()
{
	ASTNode* node = ParseStatement();

	std::vector<ASTNode*> nodes;
	nodes.push_back(node);

	// End of a statement is signified by semicolon, end of statement list is right curly bracket
	while (lexer->GetCurrentToken() == ";" || lexer->GetCurrentToken() == "}")
	{
		lexer->Consume(lexer->GetCurrentToken());
		nodes.push_back(ParseStatement());
	}

	return nodes;
}
// STATEMENT : COMPOUND_STATEMENT | ASSIGN_STATEMENT | EMPTY_STATEMENT  - statement can be compound not reflected here
ASTNode* Parser::ParseStatement()           // just if and assignment now - FOR/WHILE..etc go here
{
	ASTNode* node;
	if (lexer->GetCurrentToken() == "if") node = ParseIf();              // Handle keywords better!
	else if (lexer->GetCurrentToken() == "while") node = ParseWhile();
	else if (lexer->GetCurrentToken() == "END") node = ParseEmpty();  // hack
	else if (lexer->IsIdentifier(lexer->GetCurrentToken(), true)) node = ParseAssignStatement();  // Could also be a declaration - for later
	else node = ParseEmpty();

	return node;
}
// ASSIGN_STATEMENT := IDENTIFIER ASSIGN EXPRESSION
ASTNode* Parser::ParseAssignStatement()
{
	std::string current = lexer->GetCurrentToken();
	IdentifierNode* ident = new IdentifierNode(current);
	lexer->Consume(current);
	lexer->Consume("=");
	return new AssignStatementNode(ident, ParseExpr());
}
ASTNode* Parser::ParseEmpty() { return new EmptyStatementNode(); }  // not needed, can directly return empty

Parser::Parser(Lexer* lex) : lexer(lex)
{
	try { root = ParseProgram(); }
	catch (UnexpectedTokenException ex) { failState = true; std::cout << ex.what() << "\n"; }

	// Not all tokens were processed
	if (!lexer->Done()) { failState = true; std::cout << "Unrecognized token: '" + lexer->GetCurrentToken() + "'"; }  // meh

	if (!failState) root->Print();
	std::cin.get(); // debug only
}

Parser::~Parser()
{
	// CONSIDER SMART POINTERS
	delete root;
}

// FEATURES MISSING TODO:
/*
	MUST:
	GENERATE ASSEMBLY FROM AST
	VISITING NODES
	JSON AST VISUALISATION?
	COULD EXPAND UPON:
	FOR, FUNCTIONS, ARRAYS, MISCELLANEOUS
*/

// PARTIAL IMPLEMENTATION TODO:
/*
	PROGRAM INCOMPLETE
	NO VARIABLE DECLARATION (TYPES)
	NO TYPE CHECKING
	NO SYMBOL TABLE ??
	VARIABLE SCOPE
*/

// GRAMMARS TODO :
/*
FOR_STATEMENT := FOR LPAR (ASSIGN_STATEMENT | EMPTY_STATEMENT) (CONDITION | EMPTY_STATEMENT) (STEP | EMPTY_STATEMENT) RPAR { COMPOUND_STATEMENT }
FUNCTION := TYPE IDENTIFIER (  comma separated list of type identifiers ) { COMPOUND_STATEMENT }
ARRAYS?
*/