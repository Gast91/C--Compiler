#pragma once
#include <unordered_map>

enum class TokenID
{
// Variable/Function Names
    IDENTIFIER,
// Numeric Literals
    INT_LITERAL,
    FLOAT_LITERAL,
// Arithmetic Operators
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    INCR,
    DECR,
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
    BIT_NOT,
// Logical Operators
    AND,
    OR,
    NOT,
// Assignment Operators
    ASSIGN,
    ADD_ASGN,
    SUB_ASGN,
    MUL_ASGN,
    DIV_ASGN,
    MOD_ASGN,
    XOR_ASGN,
    SHL_ASGN,
    SHR_ASGN,
    B_OR_ASGN,
    B_AND_ASGN,
// Reserved Keywords       // Missing stuff, to be added as the parser expands - until then missing ones are treated as identifiers
    IF,
    ELSE,
    WHILE,
    DO,
    INT_TYPE,
    FLOAT_TYPE,
    RET,
    MAIN,                  // MAIN IS NOT A RESERVED WORD - ITS AN IDENTIFIER THAT CAN ONLY EXIST ONE OF FOR FUNCTIONS - HACK FOR NOW
// Terminals
    LPAR,
    RPAR,
    LCURLY,
    RCURLY,
    LBRACK,
    RBRACK,
    SEMI,
// Strings
    S_QUOTE,
    D_QUOTE,
    RAWSTR,
    ESCAPESEQ,         // ???
// Miscellaneous
    COMMA,
    TERNARY,
    DOT,
    ENDF,
    UNKNOWN
};

static std::unordered_map<std::string, TokenID> tokens =
{
//----------Arithmetic Operators-----------------
//-----------------------------------------------
    { "+"      , TokenID::ADD        },
    { "-"      , TokenID::SUB        },
    { "*"      , TokenID::MUL        },
    { "/"      , TokenID::DIV        },
    { "%"      , TokenID::MOD        },
    { "++"     , TokenID::INCR       },
    { "--"     , TokenID::DECR       },
//------------Shift Operators--------------------
//-----------------------------------------------
    { "<<"     , TokenID::SHL        },
    { ">>"     , TokenID::SHR        },
//------Relational and Equality Operations-------
//-----------------------------------------------
    { ">"      , TokenID::GT         },
    { "<"      , TokenID::LT         },
    { ">="     , TokenID::GTE        },
    { "<="     , TokenID::LTE        },
    { "=="     , TokenID::EQ         },
    { "!="     , TokenID::NEQ        },
//------------Bitwise Operators------------------
//-----------------------------------------------
    { "&"      , TokenID::BIT_AND    },
    { "^"      , TokenID::BIT_XOR    },
    { "|"      , TokenID::BIT_OR     },
    { "~"      , TokenID::BIT_NOT    },
//----------Logical Operators--------------------
//-----------------------------------------------
    { "&&"     , TokenID::AND        },
    { "||"     , TokenID::OR         },
    { "!"      , TokenID::NOT        },
//----------Assignment Operators-----------------
//-----------------------------------------------
    { "="      , TokenID::ASSIGN     },
    { "+="     , TokenID::ADD_ASGN   },
    { "-="     , TokenID::SUB_ASGN   },
    { "*="     , TokenID::MUL_ASGN   },
    { "/="     , TokenID::DIV_ASGN   },
    { "%="     , TokenID::MOD_ASGN   },
    { "^="     , TokenID::XOR_ASGN   },
    { "<<="    , TokenID::SHL_ASGN   },
    { ">>="    , TokenID::SHR_ASGN   },
    { "|="     , TokenID::B_OR_ASGN  },
    { "&="     , TokenID::B_AND_ASGN },
//----------Reserved Keywords--------------------
//-----------------------------------------------
    { "if"     , TokenID::IF         },
    { "else"   , TokenID::ELSE       },
    { "while"  , TokenID::WHILE      },
    { "do"     , TokenID::DO         },
    { "int"    , TokenID::INT_TYPE   },
    { "float"  , TokenID::FLOAT_TYPE },
    { "return" , TokenID::RET        },
    { "main"   , TokenID::MAIN       },               // MAIN IS NOT A RESERVED WORD - ITS AN IDENTIFIER THAT CAN ONLY EXIST ONE OF FOR FUNCTIONS - HACK FOR NOW
//-------------Terminals-------------------------
//-----------------------------------------------
    { "("      , TokenID::LPAR       },
    { ")"      , TokenID::RPAR       },
    { "{"      , TokenID::LCURLY     },
    { "}"      , TokenID::RCURLY     },
    { "["      , TokenID::LBRACK     },
    { "]"      , TokenID::RBRACK     },
    { ";"      , TokenID::SEMI       },
//--------------Strings--------------------------
//-----------------------------------------------
    { "'"      , TokenID::S_QUOTE    },
    { "\""     , TokenID::D_QUOTE    },
    { "R"      , TokenID::RAWSTR     },
    { "\\"     , TokenID::ESCAPESEQ  },
//--------Miscellaneous Operators----------------
//-----------------------------------------------
    { ","      , TokenID::COMMA      },
    { "?"      , TokenID::TERNARY    },
    { "."      , TokenID::DOT        },  
//-----------Miscellaneous-----------------------
//-----------------------------------------------
    { "\032"   , TokenID::ENDF       }
};

struct TokenCoords
{
    size_t line = 1;
    size_t col  = 1;
};

struct Token
{
    const std::string str;
    const TokenCoords coords;
    TokenID type = TokenID::UNKNOWN;
};