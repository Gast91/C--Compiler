#include "Parser.h"

Parser::Parser(const Lexer& lex) : lexer(lex)
{
    try { root = ParseProgram(); }
    catch (UnexpectedTokenException ex) { failState = true; std::cout << ex.what() << '\n'; }

    // Somewhere, somehow not all tokens were processed.
    if (!lexer.Done()) { failState = true; std::cout << "\nPARSER ERROR: Parsing Failure\n"; }
    else std::cout << "\nParsing Successful, AST Built";
}

Parser::~Parser() { delete root; }

// FACTOR := (ADD | SUB ) FACTOR | INTEGER | IDENTIFIER | LPAR EXPRESSION RPAR
ASTNode* Parser::ParseFactor()
{
    const auto currentToken = lexer.GetCurrentToken();
    // Just an add operator before a number literal or identifier
    if (currentToken.second == Token::ADD)
    {
        lexer.Consume(Token::ADD);
        return new UnaryOperationNode(currentToken, ParseFactor());
    }
    // Just a minus operator before a number literal or identifier (negation)
    else if (currentToken.second == Token::SUB)
    {
        lexer.Consume(Token::SUB);
        return new UnaryOperationNode(currentToken, ParseFactor());
    }
    else if (currentToken.second == Token::INT_LITERAL)
    {
        lexer.Consume(Token::INT_LITERAL);
        return new IntegerNode(currentToken.first);
    }
    else if (currentToken.second == Token::IDENTIFIER)
    {
        lexer.Consume(Token::IDENTIFIER);
        return new IdentifierNode(currentToken.first, lexer.GetLine());
    }
    else if (currentToken.second == Token::LPAR)
    {
        lexer.Consume(Token::LPAR);
        ASTNode* node = ParseExpr();
        lexer.Consume(Token::RPAR);
        return node;
    }
    else throw UnexpectedTokenException("Unexpected token '" + currentToken.first + "' at line " + lexer.GetLine());
}
// TERM := FACTOR ((MUL | DIV) FACTOR)*
ASTNode* Parser::ParseTerm()
{
    ASTNode* node = ParseFactor();

    while (lexer.GetCurrentToken().second == Token::MUL || lexer.GetCurrentToken().second == Token::DIV)
    {
        const auto currentToken = lexer.GetCurrentToken();
        lexer.Consume(lexer.GetCurrentToken().second);
        node = new BinaryOperationNode(node, currentToken, ParseFactor());
    }
    return node;
}
// EXPRESSION := TERM ((PLUS | MINUS) TERM)* <---
ASTNode* Parser::ParseExpr()
{
    ASTNode* node = ParseTerm();

    while (lexer.GetCurrentToken().second == Token::ADD || lexer.GetCurrentToken().second == Token::SUB)
    {
        const auto currentToken = lexer.GetCurrentToken();
        lexer.Consume(lexer.GetCurrentToken().second);
        node = new BinaryOperationNode(node, currentToken, ParseTerm());
    }
    return node;
}
//CONDITION := EXPRESSION (LESS | MORE) EXPRESSION   [MORE NEDDED HERE]
ASTNode* Parser::ParseCond()
{
    ASTNode* node = ParseExpr();

    while (lexer.GetCurrentToken().second == Token::LT || lexer.GetCurrentToken().second == Token::GT) // <=, >=, ==, != at least | Need to handle && || also!!
    {
        const auto currentToken = lexer.GetCurrentToken();
        lexer.Consume(lexer.GetCurrentToken().second);
        node = new BinaryOperationNode(node, currentToken, ParseExpr());
    }
    return node;
}
// IF_STATEMENT =: IF_KEY LPAR CONDITION RPAR { COMPOUND_STATEMENT }  [MORE NEEDED HERE]
ASTNode* Parser::ParseIf()
{
    // We already know the token is an if, begin parsing the statement
    lexer.Consume(Token::IF);
    lexer.Consume(Token::LPAR);
    ASTNode* conditionNode = ParseCond();
    lexer.Consume(Token::RPAR);

    // Body of If statement can be a collection of statements
    ASTNode* ifBody = ParseCompoundStatement();

    return new IfNode(conditionNode, ifBody);  // if here must have slots of multiple ifelse and 1 else, how? vector?
}
// WHILE_STATEMENT := WHILE LPAR CONDITION RPAR { COMPOUND_STATEMENT }
ASTNode* Parser::ParseWhile()
{
    lexer.Consume(Token::WHILE);
    lexer.Consume(Token::LPAR);
    ASTNode* conditionNode = ParseCond();
    lexer.Consume(Token::RPAR);

    // Body of while statement can be a collection of statements
    ASTNode* whileBody = ParseCompoundStatement();

    return new WhileNode(conditionNode, whileBody);
}
// PROGRAM := int main LPAR RPAR { COMPOUND_STATEMENT }
ASTNode* Parser::ParseProgram()                                // hacky way for only main now
{
    lexer.Consume(Token::INT_TYPE);
    lexer.Consume(Token::MAIN);                                // hack here as well
    lexer.Consume(Token::LPAR); 
    lexer.Consume(Token::RPAR);

    ASTNode* node = ParseCompoundStatement();

    return node;
}
// COMPOUND_STATEMENT := LCUR STATEMENT_LIST RCUR
ASTNode* Parser::ParseCompoundStatement()
{
    CompoundStatementNode* compound = new CompoundStatementNode();
    for (const auto& statement : ParseStatementList()) compound->Push(statement);

    return compound;
}
// STATEMENT_LIST := STATEMENT | STATEMENT SEMICOLON STATEMENT_LIST 
std::vector<ASTNode*> Parser::ParseStatementList()
{
    lexer.Consume(Token::LCURLY);
    ASTNode* node = ParseStatement();

    std::vector<ASTNode*> nodes;
    nodes.push_back(node);

    // Statement list ends at a closing curly bracket
    while (lexer.GetCurrentToken().second != Token::RCURLY) nodes.push_back(ParseStatement());
    lexer.Consume(Token::RCURLY);
    return nodes;
}
// STATEMENT : COMPOUND_STATEMENT | ASSIGN_STATEMENT | EMPTY_STATEMENT
ASTNode* Parser::ParseStatement()                                                    // FOR/OTHER STAMENTS..etc go here
{
    const auto currentToken = lexer.GetCurrentToken().second;
    if         (currentToken == Token::IF)         return ParseIf();
    else if    (currentToken == Token::WHILE)      return ParseWhile();
    else if    (currentToken == Token::RET)        return ParseReturn();
    else if    (currentToken == Token::INT_TYPE)   return ParseDeclarationStatement();   // lexer.isType()? the same will happen for all types - and functions + void
    else if    (currentToken == Token::IDENTIFIER) return ParseAssignStatement();
    else if    (currentToken == Token::RCURLY)	   return ParseEmpty();                  // Compound Statement has no body
    else if    (currentToken == Token::FILE_END)   return ParseEmpty();
    else throw UnexpectedTokenException("Encountered unexpected token '" + lexer.GetCurrentToken().first + "' at line " + lexer.GetLine());
}
// DECLARATION_STATEMENT := INT/FLOAT/.. IDENTIFIER SEMI
ASTNode* Parser::ParseDeclarationStatement()
{
    // Type Specifier is next (int, float, char etc..)
    const auto currentToken = lexer.GetCurrentToken();
    lexer.Consume(currentToken.second);

    // Next is the identifier
    IdentifierNode* ident = new IdentifierNode(lexer.GetCurrentToken().first, lexer.GetLine(), currentToken.second);
    lexer.Consume(Token::IDENTIFIER);
    lexer.Consume(Token::SEMI);
    return new DeclareStatementNode(ident, currentToken);
}
// ASSIGN_STATEMENT := IDENTIFIER ASSIGN EXPRESSION
ASTNode* Parser::ParseAssignStatement()
{
    const auto currentToken = lexer.GetCurrentToken();
    IdentifierNode* ident = new IdentifierNode(currentToken.first, lexer.GetLine());
    lexer.Consume(Token::IDENTIFIER);
    lexer.Consume(Token::ASSIGN);
    ASTNode* node = new AssignStatementNode(ident, ParseExpr());
    lexer.Consume(Token::SEMI);
    return node;
}
// RETURN_STATEMENT := RETURN EXPRESSION
ASTNode* Parser::ParseReturn()
{
    lexer.Consume(Token::RET);

    ASTNode* node = new ReturnStatementNode(ParseExpr());
    lexer.Consume(Token::SEMI);
    return node;
}

ASTNode* Parser::ParseEmpty() { return new EmptyStatementNode(); }  // OBSOLETE - REMOVE HERE AND FROM  VISITOR CLASSES FOR REMOVAL

// FEATURES MISSING TODO:
/*
    MUST:
    GENERATE ASSEMBLY FROM AST
    COULD EXPAND UPON:
    FOR, FUNCTIONS, ARRAYS, MISCELLANEOUS
*/

// PARTIAL IMPLEMENTATION TODO:
/*
    PROGRAM INCOMPLETE
    NO TYPE CHECKING
*/

// GRAMMARS TODO :
/*
NO DECL AND ASSIGN IN ONE - NOT REALLY IMPORTANT ATM
FOR_STATEMENT := FOR LPAR (ASSIGN_STATEMENT | EMPTY_STATEMENT) (CONDITION | EMPTY_STATEMENT) (STEP | EMPTY_STATEMENT) RPAR { COMPOUND_STATEMENT }
FUNCTION := TYPE IDENTIFIER (  comma separated list of type identifiers ) { COMPOUND_STATEMENT }
ARRAYS?
*/