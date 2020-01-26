#include "Parser.h"

// factor ::= (plus | minus ) factor | Number | Identifier | LPar expr RPar
ASTNode* Parser::ParseFactor()
{
	std::string current = lexer->getCurrentToken();
	if (current == "+")
	{
		lexer->consume("+");
		return new UnaryASTNode(Token::Add, ParseFactor());
	}
	else if (current == "-")
	{
		lexer->consume("-");
		return new UnaryASTNode(Token::Sub, ParseFactor());
	}
	else if (lexer->IsInteger(current))
	{
		lexer->consume(current);
		return new Integer(current);
	}
	else if (lexer->IsIdentifier(current, true))
	{
		lexer->consume(current);
		return new Identifier(current);
	}
	else if (lexer->getCurrentToken() == "(")  // some other way please
	{
		lexer->consume("(");  // preferably make it an enum somehow
		ASTNode* node = ParseExpr();
		lexer->consume(")");
		return node;
	}
	else throw UnexpectedTokenException("Unexpected token '" + current + "'");
}
ASTNode* Parser::ParseTerm()
{
	ASTNode* node = ParseFactor();  // leak?

	while (lexer->getCurrentToken() == "*" || lexer->getCurrentToken() == "/") // why is this a while
	{
		Token op;
		if (lexer->getCurrentToken() == "*")  // ????
		{
			lexer->consume("*");
			op = Token::Mul;
		}
		else if (lexer->getCurrentToken() == "/")
		{
			lexer->consume("/");
			op = Token::Div;
		}
		node = new BinaryOperator(node, op, ParseFactor()); // in here leak from previous node | use temp?
	}
	return node;
}
ASTNode* Parser::ParseExpr()
{
	ASTNode* node = ParseTerm();  // leak?

	while (lexer->getCurrentToken() == "+" || lexer->getCurrentToken() == "-") // why is this a while
	{
		Token op;
		if (lexer->getCurrentToken() == "+")  // ????
		{
			lexer->consume("+");
			op = Token::Add;
		}
		else if (lexer->getCurrentToken() == "-")
		{
			lexer->consume("-");
			op = Token::Sub;
		}
		node = new BinaryOperator(node, op, ParseTerm());  // in here leak from previous node | use temp?
	}
	return node;
}
ASTNode* Parser::ParseCond()
{
	ASTNode* node = ParseExpr();

	while (lexer->getCurrentToken() == "<" || lexer->getCurrentToken() == ">") // <=, >=, ==, != at least
	{
		Token op;
		if (lexer->getCurrentToken() == "<")
		{
			lexer->consume("<");
			op = Token::Less;
		}
		else if (lexer->getCurrentToken() == ">")
		{
			lexer->consume(">");
			op = Token::More;
		}
		// all here?
		node = new Condition(node, op, ParseExpr());
	}
	return node;
}
ASTNode* Parser::ParseIf()
{
	//this will happen if the token is an if
	lexer->consume("if");
	lexer->consume("(");
	ASTNode* node = ParseCond();
	lexer->consume(")");

	lexer->consume("{");  // compounds here?
	// body
	lexer->consume("}");
	if (lexer->getCurrentToken() == "else") lexer->consume("else");
	if (lexer->getCurrentToken() == "if")
	{
		// new if node here
		// ifelseNode = ParseIf();  || cannot be parse if cause else if does not have else if? or is it fine cause there can be another else if after or else
	}
	else if(lexer->getCurrentToken() == "{")
	{
		// just body here nothing else
		lexer->consume("}");
	}
	// else if | else - on their own?
	// return new If(condition, body, else if vector?, elsebody, Token::If);
	return new If(node, nullptr, Token::If);  // if here must have slots of multiple ifelse and 1 else, how? vector?
}
// PROGRAM := int main { COMPOUND_STATEMENT }
ASTNode* Parser::ParseProgram()  // hacky way for only main now
{
	lexer->consume("int");
	lexer->consume("main");
	lexer->consume("{");
	ASTNode* node = ParseCompoundStatement();
	lexer->consume("}");

	return node;
}
ASTNode* Parser::ParseCompoundStatement()
{
	//std::vector<ASTNode*> statements = ParseStatementList();
	// root = compound()?
	//????
}
std::vector<ASTNode*>& Parser::ParseStatementList()
{
	ASTNode* node = ParseStatement();

	std::vector<ASTNode*> nodes;
	nodes.push_back(node);

	while (lexer->getCurrentToken() == ";")
	{
		lexer->consume(";");
		nodes.push_back(ParseStatement());
	}

	/*if self.current_token.type == ID:
	self.error()*/ //???
	return nodes;
}
ASTNode* Parser::ParseStatement()  // if or assignment as of now?
{
	// if statement - needs to decide what it is
	// assign statement
	ASTNode* node;
	if (lexer->getCurrentToken() == "if") node = ParseIf();
	else if (lexer->IsIdentifier(lexer->getCurrentToken(), true)) node = ParseAssignment();
	else node = ParseEmpty(); // ???

	return node;
}
ASTNode* Parser::ParseAssignment()
{
	std::string current = lexer->getCurrentToken();
	Identifier* ident = new Identifier(current);
	lexer->consume(current);
	lexer->consume("=");
	return new AssignStatement(ident, ParseExpr());
}

Parser::Parser(Lexer* lex) : lexer(lex)
{
	// at the start the parser is expecting a type (int, float, void, etc) an identifier (variable or function name)
	// then either an = (just that?) or () (function)
	// if = then parse assignment expr
	// else parse function
	// function can have many things..
	try {
		if (lexer->getCurrentToken() == "if") root = ParseIf();
	}//root = ParseCond(); }//ParseExpr(); }
	//try { root = ParseProgram(); }  hoooopeeee!
	catch (UnexpectedTokenException ex) { failState = true; std::cout << ex.what() << "\n"; }  // does this work??

	// Not all tokens were processed
	if (lexer->tokensLeft) { failState = true; std::cout << "Unrecognized token: '" + lexer->getCurrentToken() + "'"; }  // meh

	if (!failState) root->Print();
	std::cin.get(); // debug only
}
Parser::~Parser()
{
	// CONSIDER SMART POINTERS
	delete root;
}


// notes
/*
for (  
possibly type and then definitely ident = factor ;
or ident = factor ;
or ;
then
condition -> ident (same as before) cond factor || whatever cond? ;
or ;
step (how?) ;
or ;
)
{
body
}


while ( condition ) { body }

function
type identifier( 
comma separated list of type identifiers
)

class Function : public AST  / are those things nodes? probably new trees? same as bodys of ifs etc etc
{
	name, vector<Identifiers> parameters; ??
}
*/