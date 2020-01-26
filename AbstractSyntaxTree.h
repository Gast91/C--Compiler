#pragma once

enum class Token  // Terminals (apart from operators)
{
	Ident,
	Int,
	LPar = '(',
	RPar = ')',
	Mul  = '*',
	Div  = '/',
	Sub  = '-',
	Add  = '+',
	Less = '<',
	More = '>', // { } & & | | %
	Eq   = '=',
	If   = 'if'  // ?????
};

template <typename Enumeration>
auto GetTokenValue(Enumeration const value) -> typename std::underlying_type<Enumeration>::type
{
	return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

// Base Node class
class ASTNode
{
public:
	ASTNode() {}
	virtual ~ASTNode() {}

	virtual void Print() = 0;
};
// Abstract Syntax Tree Node with one branch or leaf
class UnaryASTNode : public ASTNode
{
private:
	Token op;
	ASTNode* expr;
public:
	UnaryASTNode(Token t, ASTNode* n) : ASTNode(), op(t), expr(n) {}
	virtual ~UnaryASTNode() { delete expr; }

	virtual void Print()
	{
		std::cout << "Unary: '" << (char)GetTokenValue(op) << "' [";
		expr->Print();
		std::cout << "]";
	}
};
// Abstract Syntax Tree Node with two branches or leaves
class BinaryASTNode : public ASTNode
{
protected:
	ASTNode* left;
	ASTNode* right;
	Token op;
public:
	BinaryASTNode(ASTNode* l, Token o, ASTNode* r) : ASTNode(), left(l), op(o), right(r) {}
	virtual ~BinaryASTNode()
	{
		delete left;
		delete right;
	}

	virtual void Print() = 0;
};


// Node representing a number(integer) literal
class Integer : public ASTNode       // this doesnt have a token anymore
{
private:
	int value;
public:
	Integer(const std::string& val) : ASTNode(), value(std::stoi(val)) {}
	virtual ~Integer() {}

	virtual void Print() override { std::cout << "Int: " << value; }
};
// Node representing an identifier
class Identifier : public ASTNode    // this doesnt have a token anymore - NOT USED ATM ALSO
{
private:
	Token type;  // int atm but not used
	std::string value;
public:
	Identifier(const std::string& val) : ASTNode(), value(val) {}
	virtual ~Identifier() {}

	virtual void Print() override { std::cout << "Ident: " << value; }
};


// Node representing a binary operation (Addition, Subtraction, Multiplication or Division)
class BinaryOperator : public BinaryASTNode   // very similar print with condition
{
public:
	BinaryOperator(ASTNode* l, Token o, ASTNode* r) : BinaryASTNode(l, o, r) {}
	virtual ~BinaryOperator() {}

	virtual void Print() override
	{
		std::cout << "BinOp: '" << (char)GetTokenValue(op) << "' [L: ";
		left->Print();
		std::cout << " R: ";
		right->Print();
		std::cout << "]";
	}
};
class Condition : public BinaryASTNode   // very similar print with binaryOperator
{
public:
	Condition(ASTNode* l, Token o, ASTNode* r) : BinaryASTNode(l, o, r) {}
	virtual ~Condition() {}

	virtual void Print() override
	{
		std::cout << "COND: '" << (char)GetTokenValue(op) << "' [L: ";
		left->Print();
		std::cout << " R: ";
		right->Print();
		std::cout << "]";
	}
};

// this should inherit from a ternary ASTNode
class If : public ASTNode
{
private:
	ASTNode* condition;
	ASTNode* body;
public:
	If(ASTNode* cond, ASTNode* b, Token t) : ASTNode(), condition(cond), body(b) {}   // token unused here
	virtual ~If()
	{
		delete condition;
		if (body) delete body;
	}

	virtual void Print() override
	{
		std::cout << "IF: " << "[";
		condition->Print();
		std::cout << " BODY: ";
		if (body) body->Print();   // null atm this will be compound statements -> statement ->....all the way to empty
		std::cout << "]";
	}
};
class CompoundStatement : public ASTNode  // subclass same with some other?
{
private:
	std::vector<ASTNode*> statements;
public:
	CompoundStatement() : ASTNode() {}  // ?????
	virtual ~CompoundStatement() {}

	virtual void Print() override { for (const auto& statement : statements) statement->Print(); }  // ???
};
class AssignStatement : public BinaryASTNode
{
// Left is Identifier(variable), operator is '=' and right is an expression(which can be expanded)
public:
	AssignStatement(Identifier* ident, ASTNode* expr) : BinaryASTNode(ident, Token::Eq, expr) {}
	virtual ~AssignStatement() {}

	virtual void Print() override
	{
		std::cout << "ASSIGN: '" << (char)GetTokenValue(op) << "' [L: ";
		left->Print();
		std::cout << " R: ";
		right->Print();
		std::cout << "]";
	}
};
class EmptyStatement : public ASTNode
{
public:
	EmptyStatement() : ASTNode() {}
	virtual ~EmptyStatement() {}

	virtual void Print() override { std::cout << " "; }  // or absolutely nothing?
};

// COMPOUND STATEMENTS FOR IF BODIES ETC!

// starts with expr(token);
// lexer.eat or something goes to the next token
// compare the current token type with the passed token
// type and if they match then "eat" the current token
// and assign the next token to the self.current_token,
// otherwise raise an exception. INVALID SYNTAX

/*
EXPR := TERM (PLUS | MINUS) TERM
TERM := FACTOR MUL | DIV FACTOR
FACTOR := INTEGER | IDENTIFIER | LPAR EXPR RPAR


PROGRAM := COMPOUND_STATEMENTS ? ACTUALLY its any number of functions with one int main() {} - so it ends there
COMPOUND_STATEMENT := BODIES OF FUNCTIONS IFS FORS ETC - how to mark them?  IF_STATEMENT | FUNCTION ?
statement_list is list of 0 or more statements inside a compound
STATEMENT_LIST := STATEMENT | STATEMENT ; STATEMENT_LIST
STATEMENT :=  COMPOUND | ASSIGN_STATEMENT | empty?  [end of statement inside compound is marked by ;]
empty is empty or end of statement
ASSIGN_STATEMENT := variable ASSIGN expr  (variable type?)
variable := id ???

peek is the same as IsCompound()??
*/