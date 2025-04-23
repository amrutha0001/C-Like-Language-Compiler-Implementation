#pragma once  // Ensures the file is included only once during compilation

#include <stdint.h>  // Provides fixed-width integer types (e.g., uint8_t)
#include <stddef.h>  // Provides size_t and other standard definitions

// Enumeration of token types
enum {
    token_NAME,  // Identifier (e.g., variable names)
    token_NMBR,  // Number literal
    token_STRL,  // String literal
    token_WSPC,  // Whitespace
    token_LCOM,  // Line comment (e.g., //)
    token_BCOM,  // Block comment (e.g., /* */)
    token_LPAR,  // Left parenthesis '('
    token_RPAR,  // Right parenthesis ')'
    token_LBRA,  // Left bracket '['
    token_RBRA,  // Right bracket ']'
    token_LBRC,  // Left brace '{'
    token_RBRC,  // Right brace '}'
    token_COND,  // Conditional keyword (e.g., "if")
    token_ELIF,  // Else-if keyword (e.g., "elif")
    token_ELSE,  // Else keyword (e.g., "else")
    token_DOWH,  // Do-while keyword (e.g., "do")
    token_WHIL,  // While keyword (e.g., "while")
    token_ASSN,  // Assignment operator '='
    token_EQUL,  // Equality operator '=='
    token_NEQL,  // Inequality operator '!='
    token_LTHN,  // Less-than operator '<'
    token_GTHN,  // Greater-than operator '>'
    token_LTEQ,  // Less-than-or-equal operator '<='
    token_GTEQ,  // Greater-than-or-equal operator '>='
    token_CONJ,  // Logical AND operator '&&'
    token_DISJ,  // Logical OR operator '||'
    token_PLUS,  // Addition operator '+'
    token_MINS,  // Subtraction operator '-'
    token_MULT,  // Multiplication operator '*'
    token_DIVI,  // Division operator '/'
    token_MODU,  // Modulus operator '%'
    token_NEGA,  // Logical NOT operator '!'
    token_PRNT,  // Print keyword (e.g., "print")
    token_SCOL,  // Semicolon ';'
    token_QUES,  // Question mark '?'
    token_COLN,  // Colon ':'
    token_COUNT, // Total number of token types (used for array sizing)
    token_FBEG,  // Special token: Marks the beginning of the file
    token_FEND,  // Special token: Marks the end of the file
};

// Type definition for token types (uses uint8_t for efficiency)
typedef uint8_t token_t;

// Structure representing a token
struct token {
    const uint8_t *beg;  // Pointer to the beginning of the token in the input
    const uint8_t *end;  // Pointer to the end of the token in the input
    token_t token;       // Type of the token (from the enum above)
};

// Function prototype for the lexer
int lex(const uint8_t *, size_t, struct token **, size_t *);

// Enumeration of lexer return codes
enum {
    LEX_OK,            // Lexing completed successfully
    LEX_NOMEM,         // Memory allocation failed
    LEX_UNKNOWN_TOKEN, // Encountered an unrecognized token
};