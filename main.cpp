#include "Parser.h"
#include "ASTVisualizer.h"
#include "SemanticAnalyzer.h"
#include "CodeGenerator.h"

int main(int argc, char* argv[])
{
    // Create lexer and pass to it the file to be tokenized
#ifdef _DEBUG
    const char* filePath = "source.txt";
#else
    // Extract file path
    const char* filePath = argc > 1 ? argv[argc - 1] : "";
#endif
    Lexer lexer(filePath);
    lexer.PrintTokens();

    // Create parser, pass to it the lexer and parse input from lexer
    Parser parser(lexer);
    if (parser.Success())
    {
        // Parse Successful, print the AST
        ASTVisualizer ASTPrinter;
        ASTPrinter.PrintAST(*parser.GetAST());
        // Perform Semantic Analysis to the AST
        SemanticAnalyzer semanticAnalyzer;
        try { parser.GetAST()->Accept(semanticAnalyzer); }
        catch (const std::exception& ex) { std::cout << '\n' << ex.what(); }
        // Show symbol table info gathered by the semantic analysis

        if (semanticAnalyzer.Success())
        {
            semanticAnalyzer.PrintAnalysisInfo();
            // If semantic analysis was a success then generate intermediate code
            CodeGenerator codeGenerator;
            try 
            { 
                codeGenerator.GenerateTAC(parser.GetAST());
                codeGenerator.GenerateAssembly();
            }
            catch (const std::exception&) { std::cout << "\nINTERNAL ERROR: UNSUPPORTED OPERATION"; }
        }
    }
    std::cin.get(); // Debug Only
    _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF);  // _CrtDumpMemoryLeaks() will be called AFTER main has been exited - Debug Only!
}

/* TODO:
    Verbose mode, rather than dump everything to console anyway
*/