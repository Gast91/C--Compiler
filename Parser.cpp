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
        ASTNode* node = parsingCond ? ParseCond() : ParseExpr();
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

// BOOL_EXPR := EXPR REL_OP EXPR
ASTNode* Parser::ParseBoolExpr()
{
    ASTNode* node = ParseExpr();
    while (lexer.GetCurrentToken().second == Token::LT  || lexer.GetCurrentToken().second == Token::GT  ||
           lexer.GetCurrentToken().second == Token::LTE || lexer.GetCurrentToken().second == Token::GTE ||
           lexer.GetCurrentToken().second == Token::EQ  || lexer.GetCurrentToken().second == Token::NEQ)
    {
        const auto currentToken = lexer.GetCurrentToken();
        lexer.Consume(lexer.GetCurrentToken().second);
        node = new BinaryOperationNode(node, currentToken, ParseExpr());
    }
    return node;
}

//CONDITION := BOOL_EXPR (LOG_AND|LOG_OR) BOOL_EXPR | 
            // BOOL_EXPR (LOG_AND|LOG_OR) CONDITION
ASTNode* Parser::ParseCond()
{
    // Flag for handling parentheses when parsing factors.
    // If at the process of parsing a condition, parentheses
    // mean another condition is coming not an arithmetic expression
    parsingCond = true;
    ASTNode* node = ParseBoolExpr();

    while (lexer.GetCurrentToken().second == Token::AND || lexer.GetCurrentToken().second == Token::OR)
    {
        const auto currentToken = lexer.GetCurrentToken();
        lexer.Consume(lexer.GetCurrentToken().second);
        node = new BinaryOperationNode(node, currentToken, ParseBoolExpr());
    }
    parsingCond = false;
    return node;
}

// IF_STATEMENT =: IF_KEY LPAR CONDITION RPAR { COMPOUND_STATEMENT }  [MORE NEEDED HERE]
ASTNode* Parser::ParseIf()                      // split parseIfCondition and ParseIfStatement() will also work with IfStatementNode and IfNode (i think)
{
    // We already know the token is an if, begin parsing the statement
    lexer.Consume(Token::IF);
    lexer.Consume(Token::LPAR);
    ASTNode* conditionNode = ParseCond();
    lexer.Consume(Token::RPAR);

    // Body of If statement can be a collection of statements
    return new IfNode(conditionNode, ParseCompoundStatement());

    //IfNode* ifStatement = new IfNode(conditionNode, ParseCompoundStatement());
    //while (lexer.GetCurrentToken().second == Token::ELSE)
    //{
    //    lexer.Consume(Token::ELSE);
    //    if (lexer.GetCurrentToken().second == Token::IF)  // this is an if else
    //    {
    //        lexer.Consume(Token::IF);
    //        lexer.Consume(Token::LPAR);
    //        ASTNode* conditionNode = ParseCond();
    //        lexer.Consume(Token::RPAR);
    //        ifStatement->AddElseIf(conditionNode, ParseCompoundStatement());
    //    }
    //    else // this is just an else, process and return (cant have 2 else!)
    //    {
    //        ifStatement->elseBody = ParseCompoundStatement();
    //        return ifStatement;
    //    }
    //}
    //return ifStatement;  
}

// WHILE_STATEMENT := WHILE LPAR CONDITION RPAR { COMPOUND_STATEMENT }
ASTNode* Parser::ParseWhile()
{
    lexer.Consume(Token::WHILE);
    lexer.Consume(Token::LPAR);
    ASTNode* conditionNode = ParseCond();
    lexer.Consume(Token::RPAR);

    // Body of while statement can be a collection of statements
    return new WhileNode(conditionNode, ParseCompoundStatement());
}

// DO_WHILE_STATEMENT := DO { COMPOUND_STATEMENT } WHILE LPAR CONDITION RPAR
ASTNode* Parser::ParseDoWhile()
{
    lexer.Consume(Token::DO);
    ASTNode* bodyNode = ParseCompoundStatement();
    lexer.Consume(Token::WHILE);
    lexer.Consume(Token::LPAR);
    ASTNode* conditionNode = ParseCond();
    lexer.Consume(Token::RPAR);

    // Body of do while statement can be a collection of statements
    return new DoWhileNode(conditionNode, bodyNode);
}

// PROGRAM := int main LPAR RPAR { COMPOUND_STATEMENT }
ASTNode* Parser::ParseProgram()                                // hacky way for only main now - ParseTranslationUnit-> ParseFunction or ParseDeclaration
{
    lexer.Consume(Token::INT_TYPE);
    lexer.Consume(Token::MAIN);                                // hack here as well
    lexer.Consume(Token::LPAR); 
    lexer.Consume(Token::RPAR);

    ASTNode* node = ParseCompoundStatement();

    return node;
}

// BLOCK := { COMPOUND_STATEMENT }
// Used mainly to take care of scopes not attached to statements. Needs to be
// a seperate class to be visited by the semantic analyzer.
ASTNode* Parser::ParseStatementBlock()
{
    StatementBlockNode* compound = new StatementBlockNode();
    for (const auto& statement : ParseStatementList()) compound->Push(statement);

    return compound;
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
ASTNode* Parser::ParseStatement()                                                        // FOR/OTHER STAMENTS..etc go here
{
    const auto currentToken = lexer.GetCurrentToken().second;
    if         (currentToken == Token::IF)         return ParseIf();
    else if    (currentToken == Token::WHILE)      return ParseWhile();                  // Merge Loop Statements?
    else if    (currentToken == Token::DO)         return ParseDoWhile();
    else if    (currentToken == Token::RET)        return ParseReturn();
    else if    (currentToken == Token::INT_TYPE)   return ParseDeclarationStatement();   // lexer.isType()? the same will happen for all types - and functions + void
    else if    (currentToken == Token::IDENTIFIER) return ParseAssignStatement();
    else if    (currentToken == Token::LCURLY)	   return ParseStatementBlock();         // Specifically parses free floating statement blocks (enclosed by { })
    else if    (currentToken == Token::RCURLY)	   return ParseEmpty();
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

ASTNode* Parser::ParseEmpty() { return new EmptyStatementNode(); }  // OBSOLETE - REMOVE HERE AND FROM VISITOR CLASSES FOR REMOVAL

// COMPLETE IF'S - MAKE DECLARE ASSIGN
// FEATURES MISSING TODO:
/*
    MUST:
    GENERATE ASSEMBLY FROM TAC
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