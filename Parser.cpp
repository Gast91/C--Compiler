#include "Parser.h"

Parser::Parser(const Lexer& lex) : lexer(lex)
{
    try { root = ParseProgram(); }
    catch (UnexpectedTokenException ex) { failState = true; std::cout << ex.what() << '\n'; }

    // Somewhere, somehow not all tokens were processed.
    if (!lexer.Done()) failState = true;
    else std::cout << "\nParsing Successful, AST Built";
}

// FACTOR := (ADD | SUB ) FACTOR | INTEGER | IDENTIFIER | LPAR EXPRESSION RPAR
ASTNodePtr Parser::ParseFactor()
{
    const auto currentToken = lexer.GetCurrentToken();
    // Just an add operator before a number literal or identifier
    if (currentToken.second == Token::ADD)
    {
        lexer.Consume(Token::ADD);
        return std::make_unique<UnaryOperationNode>(currentToken, ParseFactor());
    }
    // Just a minus operator before a number literal or identifier (negation)
    else if (currentToken.second == Token::SUB)
    {
        lexer.Consume(Token::SUB);
        return std::make_unique<UnaryOperationNode>(currentToken, ParseFactor());
    }
    else if (currentToken.second == Token::INT_LITERAL)
    {
        lexer.Consume(Token::INT_LITERAL);
        return std::make_unique<IntegerNode>(currentToken.first);
    }
    else if (currentToken.second == Token::IDENTIFIER)
    {
        lexer.Consume(Token::IDENTIFIER);
        return std::make_unique<IdentifierNode>(currentToken.first, lexer.GetLine());
    }
    else if (currentToken.second == Token::LPAR)
    {
        lexer.Consume(Token::LPAR);
        ASTNodePtr node = parsingCond ? ParseCond() : ParseExpr();
        lexer.Consume(Token::RPAR);
        return node;
    }
    else throw UnexpectedTokenException("Unexpected token '" + currentToken.first + "' at line " + lexer.GetLine());
}

// TERM := FACTOR ((MUL | DIV) FACTOR)*
ASTNodePtr Parser::ParseTerm()
{
    ASTNodePtr node = ParseFactor();

    while (lexer.GetCurrentToken().second == Token::MUL || lexer.GetCurrentToken().second == Token::DIV)
    {
        const auto currentToken = lexer.GetCurrentToken();
        lexer.Consume(lexer.GetCurrentToken().second);
        node = std::make_unique<BinaryOperationNode>(std::move(node), currentToken, ParseFactor());
    }
    return node;
}

// EXPRESSION := TERM ((PLUS | MINUS) TERM)* <---
ASTNodePtr Parser::ParseExpr()
{
    ASTNodePtr node = ParseTerm();

    while (lexer.GetCurrentToken().second == Token::ADD || lexer.GetCurrentToken().second == Token::SUB)
    {
        const auto currentToken = lexer.GetCurrentToken();
        lexer.Consume(lexer.GetCurrentToken().second);
        node = std::make_unique<BinaryOperationNode>(std::move(node), currentToken, ParseTerm());
    }
    return node;
}

// BOOL_EXPR := EXPR REL_OP EXPR
ASTNodePtr Parser::ParseBoolExpr()
{
    ASTNodePtr node = ParseExpr();
    while (lexer.GetCurrentToken().second == Token::LT  || lexer.GetCurrentToken().second == Token::GT  ||
           lexer.GetCurrentToken().second == Token::LTE || lexer.GetCurrentToken().second == Token::GTE ||
           lexer.GetCurrentToken().second == Token::EQ  || lexer.GetCurrentToken().second == Token::NEQ)
    {
        const auto currentToken = lexer.GetCurrentToken();
        lexer.Consume(lexer.GetCurrentToken().second);
        node = std::make_unique<BinaryOperationNode>(std::move(node), currentToken, ParseExpr());
    }
    return node;
}

//CONDITION := BOOL_EXPR (LOG_AND|LOG_OR) BOOL_EXPR | 
            // BOOL_EXPR (LOG_AND|LOG_OR) CONDITION
ASTNodePtr Parser::ParseCond()
{
    // Flag for handling parentheses when parsing factors.
    // If at the process of parsing a condition, parentheses
    // mean another condition is coming not an arithmetic expression
    parsingCond = true;
    ASTNodePtr node = ParseBoolExpr();

    while (lexer.GetCurrentToken().second == Token::AND || lexer.GetCurrentToken().second == Token::OR)
    {
        const auto currentToken = lexer.GetCurrentToken();
        lexer.Consume(lexer.GetCurrentToken().second);
        node = std::make_unique<BinaryOperationNode>(std::move(node), currentToken, ParseBoolExpr());
    }
    parsingCond = false;
    return node;
}

ASTNodePtr Parser::ParseIfCond()
{
    lexer.Consume(Token::IF);
    lexer.Consume(Token::LPAR);
    ASTNodePtr conditionNode = ParseCond();
    lexer.Consume(Token::RPAR);
    return conditionNode;
}

// IF_STATEMENT =: IF_KEY LPAR CONDITION RPAR { COMPOUND_STATEMENT }  [MORE NEEDED HERE]
ASTNodePtr Parser::ParseIfStatement()
{
    IfStatementNodePtr ifStatement = std::make_unique<IfStatementNode>();
    ifStatement->AddNode(std::make_unique<IfNode>(ParseCompoundStatement(), ParseIfCond()));
    while (lexer.GetCurrentToken().second == Token::ELSE)  // Can be 0 or more else if's and 0 or 1 else
    {
        // Is there an else if coming?
        lexer.Consume(Token::ELSE);
        if (lexer.GetCurrentToken().second == Token::IF) 
            ifStatement->AddNode(std::make_unique<IfNode>(ParseCompoundStatement(), ParseIfCond()));
        else // So it is just an else
        {
            ifStatement->elseBody = ParseCompoundStatement();
            // Cant have more than one else
            return ifStatement;
        }
    }
    return ifStatement;
}

// WHILE_STATEMENT := WHILE LPAR CONDITION RPAR { COMPOUND_STATEMENT }
ASTNodePtr Parser::ParseWhile()
{
    lexer.Consume(Token::WHILE);
    lexer.Consume(Token::LPAR);
    ASTNodePtr conditionNode = ParseCond();
    lexer.Consume(Token::RPAR);

    // Body of while statement can be a collection of statements
    return std::make_unique<WhileNode>(std::move(conditionNode), ParseCompoundStatement());
}

// DO_WHILE_STATEMENT := DO { COMPOUND_STATEMENT } WHILE LPAR CONDITION RPAR
ASTNodePtr Parser::ParseDoWhile()
{
    lexer.Consume(Token::DO);
    ASTNodePtr bodyNode = ParseCompoundStatement();
    lexer.Consume(Token::WHILE);
    lexer.Consume(Token::LPAR);
    ASTNodePtr conditionNode = ParseCond();
    lexer.Consume(Token::RPAR);

    // Body of do while statement can be a collection of statements
    return std::make_unique<DoWhileNode>(std::move(conditionNode), std::move(bodyNode));
}

// PROGRAM := int main LPAR RPAR { COMPOUND_STATEMENT }
ASTNodePtr Parser::ParseProgram()                           // hacky way for only main now - ParseTranslationUnit-> ParseFunction or ParseDeclaration
{
    lexer.Consume(Token::INT_TYPE);
    lexer.Consume(Token::MAIN);                             // hack here as well
    lexer.Consume(Token::LPAR); 
    lexer.Consume(Token::RPAR);

    //ASTNode* node = ParseCompoundStatement();

    return ParseCompoundStatement(); //node;
}

// BLOCK := { COMPOUND_STATEMENT }
// Used mainly to take care of scopes not attached to statements. Needs to be
// a seperate class to be visited by the semantic analyzer.
ASTNodePtr Parser::ParseStatementBlock()
{
    StatementBlockNodePtr compound = std::make_unique<StatementBlockNode>();
    for (auto& statement : ParseStatementList()) compound->Push(std::move(statement));

    return compound;
}

// COMPOUND_STATEMENT := LCUR STATEMENT_LIST RCUR
ASTNodePtr Parser::ParseCompoundStatement()
{
    CompoundStatementNodePtr compound = std::make_unique<CompoundStatementNode>();
    for (auto& statement : ParseStatementList()) compound->Push(std::move(statement));

    return compound;
}

// STATEMENT_LIST := STATEMENT | STATEMENT SEMICOLON STATEMENT_LIST 
std::vector<ASTNodePtr> Parser::ParseStatementList()
{
    lexer.Consume(Token::LCURLY);
    ASTNodePtr node = ParseStatement();

    std::vector<ASTNodePtr> nodes;
    nodes.push_back(std::move(node));

    // Statement list ends at a closing curly bracket
    while (lexer.GetCurrentToken().second != Token::RCURLY) nodes.push_back(ParseStatement());
    lexer.Consume(Token::RCURLY);
    return nodes;
}

// STATEMENT : COMPOUND_STATEMENT | ASSIGN_STATEMENT | EMPTY_STATEMENT
ASTNodePtr Parser::ParseStatement()                                                        // FOR/OTHER STAMENTS..etc go here
{
    const auto currentToken = lexer.GetCurrentToken().second;
    if         (currentToken == Token::IF)         return ParseIfStatement();
    else if    (currentToken == Token::WHILE)      return ParseWhile();                  // Merge Loop Statements?
    else if    (currentToken == Token::DO)         return ParseDoWhile();
    else if    (currentToken == Token::RET)        return ParseReturn();
    else if    (currentToken == Token::INT_TYPE)   return ParseDeclarationStatement();   // lexer.isType()? the same will happen for all types - and functions + void
    else if    (currentToken == Token::IDENTIFIER) return ParseAssignStatement();        // Can parse an assign statement or a declare and assign statement
    else if    (currentToken == Token::LCURLY)	   return ParseStatementBlock();         // Specifically parses free floating statement blocks (enclosed by { })
    else if    (currentToken == Token::RCURLY)	   return ParseEmpty();
    else if    (currentToken == Token::FILE_END)   return ParseEmpty();
    else throw UnexpectedTokenException("Encountered unexpected token '" + lexer.GetCurrentToken().first + "' at line " + lexer.GetLine());
}

// DECLARATION_STATEMENT := TYPE_SPECIFIER IDENTIFIER SEMI |
                          //TYPE_SPECIFIER ASSIGN_STATEMENT
ASTNodePtr Parser::ParseDeclarationStatement()
{
    // Get the type specifier (int, float, char etc..) and consume it
    const auto typeToken = lexer.GetCurrentToken();
    lexer.Consume(typeToken.second);
    // Next is identifier so process it
    IdentifierNodePtr ident = std::make_unique<IdentifierNode>(lexer.GetCurrentToken().first, lexer.GetLine(), typeToken.second);
    lexer.Consume(Token::IDENTIFIER);
    // If there is an assignment following this is a declaration and assignment statement in one
    if (lexer.GetCurrentToken().second == Token::ASSIGN)
    {
        // Process the rest as a declare and assign statement
        lexer.Consume(Token::ASSIGN);
        DeclareAssignNodePtr node = std::make_unique<DeclareAssignNode>(std::make_unique<DeclareStatementNode>(std::move(ident), typeToken), ParseExpr());
        lexer.Consume(Token::SEMI);
        return node;
    }
    // Or is was just a declaration statement
    lexer.Consume(Token::SEMI);
    return std::make_unique<DeclareStatementNode>(std::move(ident), typeToken);
}

// ASSIGN_STATEMENT := IDENTIFIER ASSIGN EXPRESSION
ASTNodePtr Parser::ParseAssignStatement()
{
    const auto currentToken = lexer.GetCurrentToken();
    IdentifierNodePtr ident = std::make_unique<IdentifierNode>(currentToken.first, lexer.GetLine());
    lexer.Consume(Token::IDENTIFIER);
    lexer.Consume(Token::ASSIGN);
    ASTNodePtr node = std::make_unique<AssignStatementNode>(std::move(ident), ParseExpr());
    lexer.Consume(Token::SEMI);
    return node;
}

// RETURN_STATEMENT := RETURN EXPRESSION
ASTNodePtr Parser::ParseReturn()
{
    lexer.Consume(Token::RET);

    ASTNodePtr node = std::make_unique<ReturnStatementNode>(ParseExpr());
    lexer.Consume(Token::SEMI);
    return node;
}

ASTNodePtr Parser::ParseEmpty() { return std::make_unique<EmptyStatementNode>(); }

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
FOR_STATEMENT := FOR LPAR (ASSIGN_STATEMENT | EMPTY_STATEMENT) (CONDITION | EMPTY_STATEMENT) (STEP | EMPTY_STATEMENT) RPAR { COMPOUND_STATEMENT }
FUNCTION := TYPE IDENTIFIER (  comma separated list of type identifiers ) { COMPOUND_STATEMENT }
ARRAYS?
*/