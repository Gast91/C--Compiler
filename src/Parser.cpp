#include "Parser.h"

Parser::Parser(Lexer* lex) : lexer(lex) {}

void Parser::Parse()
{
    try 
    { 
        root = ParseProgram();
        done = true;
    }
    catch (UnexpectedTokenException ex) 
    { 
        failState = true; 
        Logger::Instance()->Log("%s \n", ex.what()); 
    }

    // Somewhere, somehow not all tokens were processed.
    if (!lexer->Done()) failState = true;  // - SHOW WHAT IS LEFT???
    else Logger::Instance()->Log("\nParsing Successful, AST Built");
}

// FACTOR := (ADD | SUB ) FACTOR | INTEGER | IDENTIFIER | LPAR EXPRESSION RPAR
UnqPtr<ASTNode> Parser::ParseFactor()
{
    //const auto currentToken = lexer.GetCurrentToken();
    const auto&[tokValue, tokType, line, col] = lexer->GetCurrentToken();
    // Just a unary operator (+ or -) before a literal or identifier
    if (tokType == Token::ADD || tokType == Token::SUB)
    {
        lexer->Consume(tokType);
        return std::make_unique<UnaryOperationNode>(TokenPair{ tokValue, tokType }, ParseFactor());
    }
    else if (tokType == Token::INT_LITERAL)
    {
        lexer->Consume(Token::INT_LITERAL);
        return std::make_unique<IntegerNode>(tokValue);
    }
    else if (tokType == Token::IDENTIFIER)
    {
        lexer->Consume(Token::IDENTIFIER);
        return std::make_unique<IdentifierNode>(tokValue, std::to_string(line), tokType);
    }
    else if (tokType == Token::LPAR)
    {
        lexer->Consume(Token::LPAR);
        UnqPtr<ASTNode> node = parsingCond ? ParseCond() : ParseExpr();
        lexer->Consume(Token::RPAR);
        return node;
    }
    else throw UnexpectedTokenException(lexer->GetErrorInfo(), "Encountered Unexpected Token '" + lexer->GetCurrentTokenVal() + '\'');
    //tokValue?
}

// TERM := FACTOR ((MUL | DIV) FACTOR)*
UnqPtr<ASTNode> Parser::ParseTerm()
{
    UnqPtr<ASTNode> node = ParseFactor();

    while (std::get<1>(lexer->GetCurrentToken()) == Token::MUL || std::get<1>(lexer->GetCurrentToken()) == Token::DIV)
    {
        const auto& currentToken = lexer->GetCurrentToken();
        lexer->Consume(std::get<1>(currentToken));
        node = std::make_unique<BinaryOperationNode>(std::move(node), TokenPair{ std::get<0>(currentToken), std::get<1>(currentToken) }, ParseFactor());
    }
    return node;
}

// EXPRESSION := TERM ((PLUS | MINUS) TERM)* <---
UnqPtr<ASTNode> Parser::ParseExpr()
{
    UnqPtr<ASTNode> node = ParseTerm();

    while (std::get<1>(lexer->GetCurrentToken()) == Token::ADD || std::get<1>(lexer->GetCurrentToken()) == Token::SUB)
    {
        const auto currentToken = lexer->GetCurrentToken();
        lexer->Consume(std::get<1>(currentToken));
        node = std::make_unique<BinaryOperationNode>(std::move(node), TokenPair{ std::get<0>(currentToken), std::get<1>(currentToken) }, ParseTerm());
    }
    return node;
}

// BOOL_EXPR := EXPR REL_OP EXPR
UnqPtr<ASTNode> Parser::ParseBoolExpr()
{
    UnqPtr<ASTNode> node = ParseExpr();
    while (std::get<1>(lexer->GetCurrentToken()) == Token::LT  || std::get<1>(lexer->GetCurrentToken()) == Token::GT  ||
           std::get<1>(lexer->GetCurrentToken()) == Token::LTE || std::get<1>(lexer->GetCurrentToken()) == Token::GTE ||
           std::get<1>(lexer->GetCurrentToken()) == Token::EQ  || std::get<1>(lexer->GetCurrentToken()) == Token::NEQ)
    {
        auto currentToken = lexer->GetCurrentToken();
        lexer->Consume(std::get<1>(currentToken));
        node = std::make_unique<ConditionNode>(std::move(node), TokenPair{ std::get<0>(currentToken), std::get<1>(currentToken) }, ParseExpr());  // shouldnt be a condition in the long run
    }
    return node;
}

//CONDITION := BOOL_EXPR (LOG_AND|LOG_OR) BOOL_EXPR | 
            // BOOL_EXPR (LOG_AND|LOG_OR) CONDITION
UnqPtr<ASTNode> Parser::ParseCond()
{
    // Flag for handling parentheses when parsing factors.
    // If at the process of parsing a condition, parentheses
    // mean another condition is coming not an arithmetic expression
    parsingCond = true;
    UnqPtr<ASTNode> node = ParseBoolExpr();

    while (std::get<1>(lexer->GetCurrentToken()) == Token::AND || std::get<1>(lexer->GetCurrentToken()) == Token::OR)
    {
        auto currentToken = lexer->GetCurrentToken();
        lexer->Consume(std::get<1>(currentToken));
        node = std::make_unique<ConditionNode>(std::move(node), TokenPair{ std::get<0>(currentToken), std::get<1>(currentToken) }, ParseBoolExpr());  // shouldnt be a condition in the long run
    }
    parsingCond = false;
    return node;
}

UnqPtr<ASTNode> Parser::ParseIfCond()
{
    lexer->Consume(Token::IF);
    lexer->Consume(Token::LPAR);
    UnqPtr<ASTNode> conditionNode = ParseCond();
    lexer->Consume(Token::RPAR);
    return conditionNode;
}

// IF_STATEMENT =: IF_KEY LPAR CONDITION RPAR { COMPOUND_STATEMENT }  [MORE NEEDED HERE]
UnqPtr<ASTNode> Parser::ParseIfStatement()
{
    UnqPtr<IfStatementNode> ifStatement = std::make_unique<IfStatementNode>();
    ifStatement->AddNode(std::make_unique<IfNode>(ParseCompoundStatement(), ParseIfCond()));
    while (std::get<1>(lexer->GetCurrentToken()) == Token::ELSE)  // Can be 0 or more else if's and 0 or 1 else
    {
        // Is there an else if coming?
        lexer->Consume(Token::ELSE);
        if (std::get<1>(lexer->GetCurrentToken()) == Token::IF)
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
UnqPtr<ASTNode> Parser::ParseWhile()
{
    lexer->Consume(Token::WHILE);
    lexer->Consume(Token::LPAR);
    UnqPtr<ASTNode> conditionNode = ParseCond();
    lexer->Consume(Token::RPAR);

    // Body of while statement can be a collection of statements
    return std::make_unique<WhileNode>(std::move(conditionNode), ParseCompoundStatement());
}

// DO_WHILE_STATEMENT := DO { COMPOUND_STATEMENT } WHILE LPAR CONDITION RPAR
UnqPtr<ASTNode> Parser::ParseDoWhile()
{
    lexer->Consume(Token::DO);
    UnqPtr<ASTNode> bodyNode = ParseCompoundStatement();
    lexer->Consume(Token::WHILE);
    lexer->Consume(Token::LPAR);
    UnqPtr<ASTNode> conditionNode = ParseCond();
    lexer->Consume(Token::RPAR);

    // Body of do while statement can be a collection of statements
    return std::make_unique<DoWhileNode>(std::move(conditionNode), std::move(bodyNode));
}

// PROGRAM := int main LPAR RPAR { COMPOUND_STATEMENT }
UnqPtr<ASTNode> Parser::ParseProgram()                           // hacky way for only main now - ParseTranslationUnit-> ParseFunction or ParseDeclaration
{
    lexer->Consume(Token::INT_TYPE);
    lexer->Consume(Token::MAIN);                                  // hack here as well
    lexer->Consume(Token::LPAR); 
    lexer->Consume(Token::RPAR);

    //ASTNode* node = ParseCompoundStatement();

    return ParseCompoundStatement(); //node;
}

// BLOCK := { COMPOUND_STATEMENT }
// Used mainly to take care of scopes not attached to statements. Needs to be
// a seperate class to be visited by the semantic analyzer.
UnqPtr<ASTNode> Parser::ParseStatementBlock()
{
    UnqPtr<StatementBlockNode> compound = std::make_unique<StatementBlockNode>();
    for (auto& statement : ParseStatementList()) compound->Push(std::move(statement));

    return compound;
}

// COMPOUND_STATEMENT := LCUR STATEMENT_LIST RCUR
UnqPtr<ASTNode> Parser::ParseCompoundStatement()
{
    UnqPtr<CompoundStatementNode> compound = std::make_unique<CompoundStatementNode>();
    for (auto& statement : ParseStatementList()) compound->Push(std::move(statement));

    return compound;
}

// STATEMENT_LIST := STATEMENT | STATEMENT SEMICOLON STATEMENT_LIST 
std::vector<UnqPtr<ASTNode>> Parser::ParseStatementList()
{
    lexer->Consume(Token::LCURLY);
    UnqPtr<ASTNode> node = ParseStatement();

    std::vector<UnqPtr<ASTNode>> nodes;
    nodes.push_back(std::move(node));

    // Statement list ends at a closing curly bracket
    while (std::get<1>(lexer->GetCurrentToken()) != Token::RCURLY) nodes.push_back(ParseStatement());
    lexer->Consume(Token::RCURLY);
    return nodes;
}

// STATEMENT : COMPOUND_STATEMENT | ASSIGN_STATEMENT |
// ITERATION_STATEMENT | DECL | ASSIGN | STATEMENT_BLOCK | EMPTY_STATEMENT
UnqPtr<ASTNode> Parser::ParseStatement()                                              // FOR/OTHER STAMENTS..etc go here
{
    const auto&[tokenValue, tokenType, line, col] = lexer->GetCurrentToken();
    if         (tokenType == Token::IF)         return ParseIfStatement();
    else if    (tokenType == Token::WHILE)      return ParseWhile();                  // Merge Loop Statements?
    else if    (tokenType == Token::DO)         return ParseDoWhile();
    else if    (tokenType == Token::RET)        return ParseReturn();
    else if    (tokenType == Token::INT_TYPE)   return ParseDeclarationStatement();   // lexer.isType()? the same will happen for all types - and functions + void
    else if    (tokenType == Token::IDENTIFIER) return ParseAssignStatement();        // Can parse an assign statement or a declare and assign statement
    else if    (tokenType == Token::LCURLY)	    return ParseStatementBlock();         // Specifically parses free floating statement blocks (enclosed by { })
    else if    (tokenType == Token::RCURLY)	    return ParseEmpty();
    else if    (tokenType == Token::FILE_END)   return ParseEmpty();
    else throw UnexpectedTokenException(lexer->GetErrorInfo(), "Encountered Unexpected Token '" + lexer->GetCurrentTokenVal() + '\'');
    //tokenValue?
}

// DECLARATION_STATEMENT := TYPE_SPECIFIER IDENTIFIER SEMI |
                          //TYPE_SPECIFIER ASSIGN_STATEMENT
UnqPtr<ASTNode> Parser::ParseDeclarationStatement() // in the future it should accommodate function declarations also
{
    // Get the type specifier (int, float, char etc..) and consume it
    std::string tokenVal; Token tokenType;
    std::tie(tokenVal, tokenType, std::ignore, std::ignore) = lexer->GetCurrentToken();
    lexer->Consume(tokenType);

    // Next is identifier so process it
    UnqPtr<IdentifierNode> ident = std::make_unique<IdentifierNode>(lexer->GetCurrentTokenVal(), lexer->GetCurrentTokenLine(), tokenType);
    lexer->Consume(Token::IDENTIFIER);
    // If there is an assignment following this is a declaration and assignment statement in one
    if (lexer->GetCurrentTokenType() == Token::ASSIGN)
    {
        // Process the rest as a declare and assign statement
        lexer->Consume(Token::ASSIGN);
        UnqPtr<DeclareAssignNode> node = std::make_unique<DeclareAssignNode>(std::make_unique<DeclareStatementNode>(std::move(ident), TokenPair{ tokenVal, tokenType }), ParseExpr());
        lexer->Consume(Token::SEMI);
        return node;
    }
    // Or is was just a declaration statement
    lexer->Consume(Token::SEMI);
    return std::make_unique<DeclareStatementNode>(std::move(ident), TokenPair{ tokenVal, tokenType });
}

// ASSIGN_STATEMENT := IDENTIFIER ASSIGN EXPRESSION
UnqPtr<ASTNode> Parser::ParseAssignStatement()
{
    const auto& [tokenValue, tokenType, line, col] = lexer->GetCurrentToken();
    UnqPtr<IdentifierNode> ident = std::make_unique<IdentifierNode>(tokenValue, std::to_string(line), tokenType);
    lexer->Consume(Token::IDENTIFIER);
    lexer->Consume(Token::ASSIGN);
    UnqPtr<ASTNode> node = std::make_unique<AssignStatementNode>(std::move(ident), ParseExpr());
    lexer->Consume(Token::SEMI);
    return node;
}

// RETURN_STATEMENT := RETURN EXPRESSION
UnqPtr<ASTNode> Parser::ParseReturn()
{
    lexer->Consume(Token::RET);

    UnqPtr<ASTNode> node = std::make_unique<ReturnStatementNode>(ParseExpr());
    lexer->Consume(Token::SEMI);
    return node;
}

UnqPtr<ASTNode> Parser::ParseEmpty() { return std::make_unique<EmptyStatementNode>(); }