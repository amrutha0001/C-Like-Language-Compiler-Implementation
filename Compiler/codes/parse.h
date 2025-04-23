#pragma once  // Ensure this header file is included only once during compilation

#include <stdio.h>
#include <stdint.h>  // For fixed-width integer types like uint8_t and uint32_t
#include <stddef.h>  // For size_t type

// Enumeration of node types in the abstract syntax tree (AST)
// Each constant represents a kind of syntactic construct
enum {
    NT_Unit,   // Top-level unit or root of the AST
    NT_Stmt,   // Generic statement
    NT_Assn,   // Assignment statement
    NT_Prnt,   // Print statement
    NT_Ctrl,   // Control flow statement (like if/while)
    NT_Cond,   // Condition block (if condition)
    NT_Elif,   // Else-if condition block
    NT_Else,   // Else condition block
    NT_Dowh,   // Do-while loop
    NT_Whil,   // While loop
    NT_Atom,   // Atomic expression or leaf node (like identifiers, numbers)
    NT_Expr,   // General expression
    NT_Pexp,   // Parenthesized expression
    NT_Bexp,   // Binary expression
    NT_Uexp,   // Unary expression
    NT_Texp,   // Ternary expression
    NT_Aexp,   // Array expression (or potentially arithmetic expression)
    NT_COUNT   // Total number of node types (not an actual node type)
};

// Define nt_t as an 8-bit unsigned integer to represent node types
typedef uint8_t nt_t;

// Forward declaration of the "token" structure
struct token;

// Structure representing a node in the abstract syntax tree (AST)
struct node {
    // Number of children nodes in this tree node
    // If nchildren == 0, it's a leaf node holding a token
    uint32_t nchildren;

    union {
        // If this is a leaf node (nchildren == 0), it holds a pointer to a token
        const struct token *token;

        // If this is a non-leaf node (nchildren > 0), it holds:
        struct {
            nt_t nt;                // Node type (one of the NT_ enum values)
            struct node **children; // Array of pointers to child nodes
        };
    };
};

// Function to parse an array of tokens into an abstract syntax tree
// Parameters:
//   - const struct token *: pointer to the array of tokens
//   - size_t: number of tokens in the array
// Returns:
//   - struct node: the root node of the parsed abstract syntax tree
struct node parse(const struct token *, size_t,FILE*);

// Possible return codes for parsing or other operations
enum {
    PARSE_OK,      // Parsing was successful
    PARSE_REJECT,  // Input was syntactically invalid (parse rejected)
    PARSE_NOMEM,   // Memory allocation failed during parsing
};

// Helper macro to determine if parsing was successful or resulted in an error
// If the root node has children, parsing was successful (PARSE_OK)
// Otherwise, it returns the token that caused the parse error
#define parse_error(root) ({ \
    struct node root_once = (root); \
    root_once.nchildren ? PARSE_OK : root_once.token->token; \
})

// Function to simplify or optimize an AST by collapsing nodes where possible
// It takes a root node and modifies the tree in place
void collapse_tree(struct node);
