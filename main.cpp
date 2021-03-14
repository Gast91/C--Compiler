#include "Parser.h"
#include "Symbol.h"
#include "ASTVisualizer.h"
#include "CodeGenerator.h"

int main(int argc, char* argv[])
{
	// Create lexer and pass to it the file to be tokenized
	Lexer* lexer = new Lexer(argv[argc - 1]);                // file hardcoded in the lexer atm
	// Show tokenized input - Debug
	lexer->PrintTokens();

	// Create parser, pass to it the lexer and parse input from lexer
	Parser* parser = new Parser(*lexer);
	if (parser->Success())
	{
		// Parse Successful, print the AST
		ASTVisualizer ASTPrinter;
		ASTPrinter.PrintAST(*(parser->GetAST()));
		// Perform Semantic Analysis to the AST
		SemanticAnalyzer* semanticAnalyzer = new SemanticAnalyzer();
		try { parser->GetAST()->Accept(*semanticAnalyzer); }
		catch (const std::exception& ex) { std::cout << "\n" << ex.what(); }
		// Show symbol table info gathered by the semantic analysis
		semanticAnalyzer->Print();
		delete semanticAnalyzer;  // we will probably need it for code generation so no destruction

		// If semantic analysis was a success
		CodeGenerator codeGenerator;
		codeGenerator.GenerateAssembly(parser->GetAST());
	}
	std::cin.get(); // Debug Only

	// Cleanup
	delete lexer;
	delete parser;

	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF);  // _CrtDumpMemoryLeaks() will be called AFTER main has been exited
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