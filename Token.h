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
// Shift Operators - >>, <<

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
// Assignment Operators +=, -=, *=, /=, %=, <<=, >>=, &=, ^=, |=
	ASSIGN,
// Reserved Keywords
	IF,
	WHILE,
	INT_TYPE,
	RET,
	MAIN,                 // MAIN IS NOT A RESERVED WORD - ITS AN IDENTIFIER THAT CAN ONLY EXIST ONE OF FOR FUNCTIONS - HACK FOR NOW
// Terminals
	LPAR,
	RPAR,
	LCURLY,
	RCURLY,
	SEMI,
// Miscellaneous
	NLINE,
	FILE_END,
	UNKNOWN,
};

static std::unordered_map<std::string, Token> tokens =
{
//-------Arithmetic Operators--------------------
//-----------------------------------------------
	{ "+"	   , Token::ADD		 },
	{ "-"	   , Token::SUB		 },
	{ "*"	   , Token::MUL		 },
	{ "/"	   , Token::DIV		 },
	{ "%"      , Token::MOD      },
//-------Shift Operators-------------------------
//-----------------------------------------------

//-------Relational and Equality Operations
//-----------------------------------------------
	{ ">"	   , Token::GT		 },
	{ "<"	   , Token::LT		 },
	{ ">="	   , Token::GTE		 },
	{ "<="	   , Token::LTE		 },
	{ "=="	   , Token::EQ		 },
	{ "!="	   , Token::NEQ		 },
//-------Bitwise Operators-----------------------
//-----------------------------------------------
	{ "&"	   , Token::BIT_AND  },
	{ "^"	   , Token::BIT_XOR  },
	{ "|"	   , Token::BIT_OR   },
//-------Logical Operators-----------------------
//-----------------------------------------------
	{ "&&"	   , Token::AND      },
	{ "||"	   , Token::OR       },
//-------Assignment Operators--------------------
//-----------------------------------------------
	{ "="	   , Token::ASSIGN   },
//-------Reserved Keywords-----------------------
//-----------------------------------------------
    { "if"     , Token::IF       },
    { "while"  , Token::WHILE    },
	{ "int"    , Token::INT_TYPE },
	{ "return" , Token::RET		 },
    { "main"   , Token::MAIN     },                  // MAIN IS NOT A RESERVED WORD - ITS AN IDENTIFIER THAT CAN ONLY EXIST ONE OF FOR FUNCTIONS - HACK FOR NOW
//-------Terminals
//-----------------------------------------------
	{ "("	   , Token::LPAR     },
	{ ")"      , Token::RPAR     },
	{ "{"	   , Token::LCURLY	 },
	{ "}"	   , Token::RCURLY	 },
	{ ";"	   , Token::SEMI	 },
//-------Miscellaneous---------------------------
//-----------------------------------------------
	{ "\n"	   , Token::NLINE	 },
	{ "\032"   , Token::FILE_END },
};

using TokenPair = std::pair<const std::string, Token>;