# C--Compiler

C--Compiler is an attempt to make, by hand, a very basic compiler to translate C++ into assembly.
The aim to begin with, is for the compiler to be able to process a relatively small subset of C++, hence 
the name C--. This project has nothing to do with the C-- proramming language despite what the name of this project might suggest. This project is part of a final year dissertation.

## Getting Started


### Prerequisites

-For the ability to visualize the abstract syntax tree, the library treant-js is required - [treant-js](https://github.com/fperucic     /treant-js).

-To compile a C++17 version is required.

### Current Features

-Lexical Analysis (Mostly Complete, some tokens, while they will be tokenized, they wont be recongized leading the parser to throw an error.)

-Parsing (Declaration, Declaration Assignment, If-elseif-else, while-dowhile, arithmetic-logical expressions, return statements)

-Semantic Analysis (Checks for variable being declared or redefined in the current scope, nested scopes, symbols, symbol tables)

-Intermediate code generation (TAC) for the language constructs mentioned above.

### In Progress

-Assembly Generation

-Various other improvements and additions to the features mentioned above, removal of limitations mentioned below or other language features that are present in C++ but the current project version can not deal with.

-Addition of Unit Tests

### Current Limitations

-If-elseif-else, while-dowhile inner statements must be enclosed by {} even if they only contain a single line

-Keywords not added to the lexer's token table will be categorized as identifiers allowing the variables declarations or assignments in the form of * **int throw = 5**.

-At the moment everything must be written inside the main function meaning that there is no function parsing, global variables etc.

-No type checking at the moment. Only available type is integers and the use of other C++ built-in types will lead the parser to complain.

-Intermediate code generation on some occasions use more temporary variables than needed.

-No intermediate code instruction for jumping to the end after a return statement.

-No plans at the moment for linking and the ability to process more than one translation unit, code optimization and C++ standards.

## Authors

* **Dimitrios Kazakos** - *Initial work* - [Gast91](https://github.com/Gast91)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
