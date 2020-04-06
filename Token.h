#pragma once
#include <unordered_map>

/* IDENTIFIER and LITERALS are not present in the tokens map at the start.
   They will be added by the lexer once encountered and identified as such
   in the form of { NAME||VALUE, IDENTIFIER||LITERAL }.
   Tokens that cannot be identified, (i.e not an identifier or literal and not present in the map)
   will be inserted into the map in the form of { VALUE , UNKNOWN } */
enum class Token
{
    IDENTIFIER,
// Literals
    INT_LITERAL,
// Arithmetic Operators
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
// Shift Operators
    SHL,
    SHR,
// Relational and Equality Operations
    GT,
    LT,
    GTE,
    LTE,
    EQ,
    NEQ,
// Bitwise Operators
    BIT_AND,
    BIT_XOR,
    BIT_OR,
// Logical Operators
    AND,
    OR,
// Assignment Operators    // MISSING <<=, >>= Lexer can't deal with triple character operators but parser (for now) cant understand them anyway
    ASSIGN,
    ADD_ASSIGN,
    SUB_ASSIGN,
    MUL_ASSIGN,
    DIV_ASSIGN,
    MOD_ASSIGN,
    XOR_ASSIGN,
    B_OR_ASSIGN,
    B_AND_ASSIGN,
// Reserved Keywords       // Missing stuff, to be added as the parser expands - until then missing ones are treated as identifiers
    IF,
    WHILE,
    DO,
    INT_TYPE,
    RET,
    MAIN,                  // MAIN IS NOT A RESERVED WORD - ITS AN IDENTIFIER THAT CAN ONLY EXIST ONE OF FOR FUNCTIONS - HACK FOR NOW
// Terminals
    LPAR,
    RPAR,
    LCURLY,
    RCURLY,
    SEMI,
// Miscellaneous
    NLINE,
    FILE_END,
    UNKNOWN
};

static std::unordered_map<std::string, Token> tokens =
{
//-------Arithmetic Operators--------------------
//-----------------------------------------------
    { "+"      , Token::ADD          },
    { "-"      , Token::SUB          },
    { "*"      , Token::MUL          },
    { "/"      , Token::DIV          },
    { "%"      , Token::MOD          },
//-------Shift Operators-------------------------
//-----------------------------------------------
    { "<<"     , Token::SHL          },
    { ">>"     , Token::SHR          },
//-------Relational and Equality Operations
//-----------------------------------------------
    { ">"      , Token::GT           },
    { "<"      , Token::LT           },
    { ">="     , Token::GTE          },
    { "<="     , Token::LTE          },
    { "=="     , Token::EQ           },
    { "!="     , Token::NEQ          },
//-------Bitwise Operators-----------------------
//-----------------------------------------------
    { "&"      , Token::BIT_AND      },
    { "^"      , Token::BIT_XOR      },
    { "|"      , Token::BIT_OR       },
//-------Logical Operators-----------------------
//-----------------------------------------------
    { "&&"     , Token::AND          },
    { "||"     , Token::OR           },
//-------Assignment Operators--------------------
//-----------------------------------------------
    { "="      , Token::ASSIGN       },
    { "+="     , Token::ADD_ASSIGN   },
    { "-="     , Token::SUB_ASSIGN   },
    { "*="     , Token::MUL_ASSIGN   },
    { "/="     , Token::DIV_ASSIGN   },
    { "%="     , Token::MOD_ASSIGN   },
    { "^="     , Token::XOR_ASSIGN   },
    { "|="     , Token::B_OR_ASSIGN  },
    { "&="     , Token::B_AND_ASSIGN },
//-------Reserved Keywords-----------------------
//-----------------------------------------------
    { "if"     , Token::IF           },
    { "while"  , Token::WHILE        },
    { "do"     , Token::DO           },
    { "int"    , Token::INT_TYPE     },
    { "return" , Token::RET          },
    { "main"   , Token::MAIN         },              // MAIN IS NOT A RESERVED WORD - ITS AN IDENTIFIER THAT CAN ONLY EXIST ONE OF FOR FUNCTIONS - HACK FOR NOW
//-------Terminals-------------------------------
//-----------------------------------------------
    { "("      , Token::LPAR         },
    { ")"      , Token::RPAR         },
    { "{"      , Token::LCURLY       },
    { "}"      , Token::RCURLY       },
    { ";"      , Token::SEMI         },
//-------Miscellaneous---------------------------
//-----------------------------------------------
    { "\n"     , Token::NLINE        },
    { "\032"   , Token::FILE_END     }
};

using TokenPair = std::pair<const std::string, Token>;