#pragma once  // Ensure this header file is only included once during compilation
#include <stdio.h>
// Forward declaration of the "node" structure
// This likely represents a node in an Abstract Syntax Tree (AST)
struct node;

// Function declaration: run
// Executes or interprets the Abstract Syntax Tree starting from the given node.
// Parameters:
//   - const struct node *: a pointer to the root of the AST to run
// The function likely traverses and evaluates the AST to perform the program's actions.
void run(const struct node *,FILE*);
