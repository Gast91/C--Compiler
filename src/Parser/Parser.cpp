#include "Parser.h"
#include "../Util/Logger.h"

void Parser::Update()
{
    if (!shouldRun) return;
    failState = false;
    try 
    { 
        // Don't bother creating the AST if there are no tokens
        root = lexer->HasTokens() ? ParseProgram() : UnqPtr<ASTNode>();
        shouldRun = false;
    }
    catch (const UnexpectedTokenException& ex)
    { 
        failState = true;
        root.reset();
        Logger::Error("{}\n", ex.what());
    }

    if (!failState && root) Logger::Info("Parsing Successful, AST Built\n");
    // Somewhere, somehow not all tokens were processed.
    if (!lexer->Done()) 
        Logger::Error("Unproccessed tokens left starting at {}:{}\n", lexer->GetCurrentTokenLine(), lexer->GetCurrentTokenCol());

    // AST was either reset or recreated, notify any observers that it has changed
    NotifyObservers(Notify::ASTChanged);

    // Reset lexer index back to the start for the next parse
    // If lexer is always called before parser (as it should), this is pointless
    lexer->ResetIndex();
}

void Parser::Reset()
{
    failState = false;
    root.reset();
    NotifyObservers(Notify::ASTChanged);
}

// FACTOR := (ADD | SUB ) FACTOR | INTEGER | IDENTIFIER | LPAR EXPRESSION RPAR
UnqPtr<ASTNode> Parser::ParseFactor()
{
    const auto&[tokValue, coords, tokType] = lexer->GetCurrentToken();
    const auto& currentToken = lexer->GetCurrentToken();
    // Just a unary operator (+ or -) before a literal or identifier
    if (tokType == TokenID::ADD || tokType == TokenID::SUB)
    {
        lexer->Consume(tokType);
        return std::make_unique<UnaryOperationNode>(currentToken, ParseFactor());
    }
    else if (tokType == TokenID::INT_LITERAL)
    {
        lexer->Consume(TokenID::INT_LITERAL);
        return std::make_unique<IntegerNode>(tokValue);
    }
    else if (tokType == TokenID::IDENTIFIER)
    {
        lexer->Consume(TokenID::IDENTIFIER);
        return std::make_unique<IdentifierNode>(currentToken);
    }
    else if (tokType == TokenID::LPAR)
    {
        lexer->Consume(TokenID::LPAR);
        UnqPtr<ASTNode> node = parsingCond ? ParseCond() : ParseExpr();
        lexer->Consume(TokenID::RPAR);
        return node;
    }
    else throw UnexpectedTokenException(lexer->GetCurrentToken(), GetSourceLine ? GetSourceLine(coords.line) : "");
}

// TERM := FACTOR ((MUL | DIV) FACTOR)*
UnqPtr<ASTNode> Parser::ParseTerm()
{
    UnqPtr<ASTNode> node = ParseFactor();

    while (lexer->GetCurrentTokenType() == TokenID::MUL || lexer->GetCurrentTokenType() == TokenID::DIV)
    {
        const auto& token = lexer->GetCurrentToken();
        lexer->Consume(token.type);
        node = std::make_unique<BinaryOperationNode>(std::move(node), token, ParseFactor());
    }
    return node;
}

// EXPRESSION := TERM ((PLUS | MINUS) TERM)* <---
UnqPtr<ASTNode> Parser::ParseExpr()
{
    UnqPtr<ASTNode> node = ParseTerm();

    while (lexer->GetCurrentTokenType() == TokenID::ADD || lexer->GetCurrentTokenType() == TokenID::SUB)
    {
        const auto& token = lexer->GetCurrentToken();
        lexer->Consume(token.type);
        node = std::make_unique<BinaryOperationNode>(std::move(node), token, ParseTerm());
    }
    return node;
}

// BOOL_EXPR := EXPR REL_OP EXPR
UnqPtr<ASTNode> Parser::ParseBoolExpr()
{
    UnqPtr<ASTNode> node = ParseExpr();
    while (lexer->GetCurrentTokenType() == TokenID::LT  || lexer->GetCurrentTokenType() == TokenID::GT  ||
           lexer->GetCurrentTokenType() == TokenID::LTE || lexer->GetCurrentTokenType() == TokenID::GTE ||
           lexer->GetCurrentTokenType() == TokenID::EQ  || lexer->GetCurrentTokenType() == TokenID::NEQ)
    {
        const auto& token = lexer->GetCurrentToken();
        lexer->Consume(token.type);
        node = std::make_unique<ConditionNode>(std::move(node), token, ParseExpr());  // shouldnt be a condition in the long run
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

    while (lexer->GetCurrentTokenType() == TokenID::AND || lexer->GetCurrentTokenType() == TokenID::OR)
    {
        const auto& token = lexer->GetCurrentToken();
        lexer->Consume(token.type);
        node = std::make_unique<ConditionNode>(std::move(node), token, ParseBoolExpr());  // shouldnt be a condition in the long run
    }
    parsingCond = false;
    return node;
}

UnqPtr<ASTNode> Parser::ParseIfCond()
{
    lexer->Consume(TokenID::IF);
    lexer->Consume(TokenID::LPAR);
    UnqPtr<ASTNode> conditionNode = ParseCond();
    lexer->Consume(TokenID::RPAR);
    return conditionNode;
}

// IF_STATEMENT =: IF_KEY LPAR CONDITION RPAR { COMPOUND_STATEMENT }  [MORE NEEDED HERE]
UnqPtr<ASTNode> Parser::ParseIfStatement()
{
    UnqPtr<IfStatementNode> ifStatement = std::make_unique<IfStatementNode>();
    ifStatement->AddNode(std::make_unique<IfNode>(ParseCompoundStatement(), ParseIfCond()));
    while (lexer->GetCurrentTokenType() == TokenID::ELSE)  // Can be 0 or more else if's and 0 or 1 else
    {
        // Is there an else if coming?
        lexer->Consume(TokenID::ELSE);
        if (lexer->GetCurrentTokenType() == TokenID::IF)
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
    lexer->Consume(TokenID::WHILE);
    lexer->Consume(TokenID::LPAR);
    UnqPtr<ASTNode> conditionNode = ParseCond();
    lexer->Consume(TokenID::RPAR);

    // Body of while statement can be a collection of statements
    return std::make_unique<WhileNode>(std::move(conditionNode), ParseCompoundStatement());
}

// DO_WHILE_STATEMENT := DO { COMPOUND_STATEMENT } WHILE LPAR CONDITION RPAR
UnqPtr<ASTNode> Parser::ParseDoWhile()
{
    lexer->Consume(TokenID::DO);
    UnqPtr<ASTNode> bodyNode = ParseCompoundStatement();
    lexer->Consume(TokenID::WHILE);
    lexer->Consume(TokenID::LPAR);
    UnqPtr<ASTNode> conditionNode = ParseCond();
    lexer->Consume(TokenID::RPAR);

    // Body of do while statement can be a collection of statements
    return std::make_unique<DoWhileNode>(std::move(conditionNode), std::move(bodyNode));
}

// PROGRAM := int main LPAR RPAR { COMPOUND_STATEMENT }
UnqPtr<ASTNode> Parser::ParseProgram()                            // hacky way for only main now - ParseTranslationUnit-> ParseFunction or ParseDeclaration
{
    lexer->Consume(TokenID::INT_TYPE);
    lexer->Consume(TokenID::MAIN);                                // hack here as well
    lexer->Consume(TokenID::LPAR); 
    lexer->Consume(TokenID::RPAR);

    return ParseCompoundStatement();
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
    lexer->Consume(TokenID::LCURLY);
    UnqPtr<ASTNode> node = ParseStatement();

    std::vector<UnqPtr<ASTNode>> nodes;
    nodes.push_back(std::move(node));

    // Statement list ends at a closing curly bracket
    while (lexer->GetCurrentTokenType() != TokenID::RCURLY) nodes.push_back(ParseStatement());
    lexer->Consume(TokenID::RCURLY);
    return nodes;
}

// STATEMENT : COMPOUND_STATEMENT | ASSIGN_STATEMENT |
// ITERATION_STATEMENT | DECL | ASSIGN | STATEMENT_BLOCK | EMPTY_STATEMENT
UnqPtr<ASTNode> Parser::ParseStatement()                                               // FOR/OTHER STAMENTS..etc go here
{
    const auto&[tokenValue, coords, tokenType] = lexer->GetCurrentToken();
    if         (tokenType == TokenID::IF)         return ParseIfStatement();
    else if    (tokenType == TokenID::WHILE)      return ParseWhile();                  // Merge Loop Statements?
    else if    (tokenType == TokenID::DO)         return ParseDoWhile();
    else if    (tokenType == TokenID::RET)        return ParseReturn();
    else if    (tokenType == TokenID::INT_TYPE)   return ParseDeclarationStatement();   // lexer.isType()? the same will happen for all types - and functions + void
    else if    (tokenType == TokenID::IDENTIFIER) return ParseAssignStatement();        // Can parse an assign statement or a declare and assign statement
    else if    (tokenType == TokenID::LCURLY)	  return ParseStatementBlock();         // Specifically parses free floating statement blocks (enclosed by { })
    else if    (tokenType == TokenID::RCURLY)     return ParseEmpty();
    else if    (tokenType == TokenID::SEMI)       // Just an empty statement (semicolon on its own)
    { 
        lexer->Consume(TokenID::SEMI); 
        return ParseEmpty();
    }
                                                
    throw UnexpectedTokenException(lexer->GetCurrentToken(), GetSourceLine ? GetSourceLine(coords.line) : "");
}

// DECLARATION_STATEMENT := TYPE_SPECIFIER IDENTIFIER SEMI |
                          //TYPE_SPECIFIER ASSIGN_STATEMENT
UnqPtr<ASTNode> Parser::ParseDeclarationStatement() // in the future it should accommodate function declarations also
{
    // Get the type specifier (int, float, char etc..) and consume it
    const auto& [tokenValue, coords, tokenType] = lexer->GetCurrentToken();
    const auto& currentTokenInfo = lexer->GetCurrentToken();
    lexer->Consume(tokenType);

    // Next is identifier so process it
    UnqPtr<IdentifierNode> ident = std::make_unique<IdentifierNode>(lexer->GetCurrentToken());
    lexer->Consume(TokenID::IDENTIFIER);
    // If there is an assignment following this is a declaration and assignment statement in one
    if (lexer->GetCurrentTokenType() == TokenID::ASSIGN)
    {
        const auto& assignTok = lexer->GetCurrentToken();
        // Process the rest as a declare and assign statement
        lexer->Consume(TokenID::ASSIGN);
        UnqPtr<DeclareAssignNode> node = 
            std::make_unique<DeclareAssignNode>(
                std::make_unique<DeclareStatementNode>(std::move(ident), currentTokenInfo),
                assignTok,
                ParseExpr());
        lexer->Consume(TokenID::SEMI);
        return node;
    }
    // Or is was just a declaration statement
    lexer->Consume(TokenID::SEMI);
    return std::make_unique<DeclareStatementNode>(std::move(ident), currentTokenInfo);
}

// ASSIGN_STATEMENT := IDENTIFIER ASSIGN EXPRESSION
UnqPtr<ASTNode> Parser::ParseAssignStatement()
{
    UnqPtr<IdentifierNode> ident = std::make_unique<IdentifierNode>(lexer->GetCurrentToken());
    lexer->Consume(TokenID::IDENTIFIER);
    const auto& assignTok = lexer->GetCurrentToken();
    lexer->Consume(TokenID::ASSIGN);
    UnqPtr<ASTNode> node = std::make_unique<AssignStatementNode>(std::move(ident), assignTok, ParseExpr());
    lexer->Consume(TokenID::SEMI);
    return node;
}

// RETURN_STATEMENT := RETURN EXPRESSION
UnqPtr<ASTNode> Parser::ParseReturn()
{
    lexer->Consume(TokenID::RET);

    UnqPtr<ASTNode> node = std::make_unique<ReturnStatementNode>(ParseExpr());
    lexer->Consume(TokenID::SEMI);
    return node;
}

UnqPtr<ASTNode> Parser::ParseEmpty() { return std::make_unique<EmptyStatementNode>(); }