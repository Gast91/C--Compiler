#include "Parser.h"

int main(int argc, char* argv[])
{
	// Create lexer and pass to it the file to be tokenized
	Lexer* lexer = new Lexer(argv[argc - 1]);
	// Show tokenized input - debug
	lexer->printTokenizedInput();
	// Create parser, pass it to the lexer and parse input from lexer
	Parser* parser = new Parser(lexer);
	
	// Use AST created by parser to output assembly
	// Assembly to executable via vendor assembler?

	// Cleanup
	delete lexer;
	delete parser;

	_CrtDumpMemoryLeaks();
}

/*---------------GRAMMAR SPECIFICATION--------------------------------------
AREXPR := NUMBER + MATHOPER + NUMBER | NUMBER + MATHOPER + AREXPR
STATEMENT := IDENT + CONDITION ...
CONDITION := IDENT + '(' + AREXPR + ')' |
EXPR??

DIGIT := '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'
INT := DIGIT | INT

MATHOPER := '+' | '-' | '*' | '/' | COMPMATHOP
COMPMATHOP := "+=" | "-=" | "*=" | "/=" | "++" | "--"   ///// SPLIT AGAIN?
LOGOPER := '>' | '<' | COMPLOGOPER
COMPLOGOPER := ">=" | "<=" | "==" | "!="
EXPRESSION := ARITHMEXPR | LOGEXPR | ASSIGNEXPR
LOGEXPR := 
ARITHMEXPR := INT MATHOPER INT | INT MATHOPER ARITHMEXPR | IDENTIFIER MATHOPER INT  ???
ASSIGNEXPR := IDENTIFIER = INT | IDENTIFIER = IDENTIFIER ;

KEYWORD := TYPE | ..
CHARACTER := begins with any lower or upper case letter, can have numbers after and "_"
IDENTIFIER := CHARACTER | IDENTIFIER
TYPE :=
DECLARESTATEMENT := TYPE ASSIGNEXPR;
*/