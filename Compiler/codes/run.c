#include "lex.h"
#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

// Forward declarations of helper functions
static void run_statement(const struct node *const, FILE *);
static void run_assign(const struct node *const, FILE *);
static void run_print(const struct node *const, FILE *);
static void run_ctrl(const struct node *const, FILE *);
static int eval_atom(const struct node *const, FILE *);
static int eval_expr(const struct node *const, FILE *);
static int eval_pexp(const struct node *const, FILE *);
static int eval_bexp(const struct node *const, FILE *);
static int eval_uexp(const struct node *const, FILE *);
static int eval_texp(const struct node *const, FILE *);
static int eval_aexp(const struct node *const, FILE *);

// Maximum number of variables that can be stored
#define VARSTORE_CAPACITY 128

// Global variable storage structure
static struct {
    size_t size;  // Current number of variables stored

    struct {
        const uint8_t *beg;        // Pointer to variable name start
        ptrdiff_t len;             // Length of variable name
        size_t array_size;        // Size of array (0 for scalar)
        int *values;              // Pointer to array values
    } vars[VARSTORE_CAPACITY];    // Array of variable entries
} varstore;

// Main execution function that runs the program unit
void run(const struct node *const unit, FILE *output_file)
{
    // Execute each statement in the unit (skipping first and last children which are likely delimiters)
    for (size_t stmt_idx = 1; stmt_idx < unit->nchildren - 1; ++stmt_idx) {
        run_statement(unit->children[stmt_idx], output_file);
    }

    // Clean up allocated memory for variables
    for (size_t var_idx = 0; var_idx < varstore.size; ++var_idx) {
        free(varstore.vars[var_idx].values);
    }

    varstore.size = 0;  // Reset variable store
}

// Execute a single statement
static void run_statement(const struct node *const stmt, FILE *output_file)
{
    // Determine statement type and delegate to appropriate handler
    switch (stmt->children[0]->nt) {
    case NT_Assn:  // Assignment statement
        run_assign(stmt->children[0], output_file);
        break;

    case NT_Prnt:  // Print statement
        run_print(stmt->children[0], output_file);
        break;

    case NT_Ctrl:  // Control flow statement
        run_ctrl(stmt->children[0], output_file);
        break;

    default:
        abort();  // Unknown statement type
    }
}

// Execute an assignment statement
static void run_assign(const struct node *const assn, FILE *output_file)
{
    // Check if left-hand side is an array access (aexp) or scalar
    const int lhs_is_aexp = assn->children[0]->nchildren;
    
    // Get array index if this is an array assignment, otherwise use 0
    const int array_idx = lhs_is_aexp ?
        eval_expr(assn->children[0]->children[2], output_file) : 0;

    // Get variable name start and length
    const uint8_t *const beg = lhs_is_aexp ?
        assn->children[0]->children[0]->token->beg :
        assn->children[0]->token->beg;

    const ptrdiff_t len = lhs_is_aexp ?
        assn->children[0]->children[0]->token->end - beg :
        assn->children[0]->token->end - beg;

    // Look for existing variable
    size_t var_idx;
    for (var_idx = 0; var_idx < varstore.size; ++var_idx) {
        if (varstore.vars[var_idx].len == len &&
            !memcmp(varstore.vars[var_idx].beg, beg, len)) {

            const size_t array_size = varstore.vars[var_idx].array_size;

            if (!array_size) {
                // Previous allocation failed
                fprintf(output_file, "WARN: a previous reallocation has failed, "
                    "assignment has no effect\n");
                return;
            }

            if (array_idx >= 0 && array_idx < array_size) {
                // Existing array/slot - just assign
                varstore.vars[var_idx].values[array_idx] =
                    eval_expr(assn->children[2], output_file);
                return;
            } else if (array_idx >= 0) {
                // Need to resize array
                const size_t new_size = (array_idx + 1) * 2;  // Double the needed size

                int *const tmp = realloc(
                    varstore.vars[var_idx].values, new_size * sizeof(int));

                if (!tmp) {
                    free(varstore.vars[var_idx].values);
                    varstore.vars[var_idx].array_size = 0;
                    varstore.vars[var_idx].values = NULL;
                    fprintf(output_file, "realloc failed\n");
                    return;
                }

                varstore.vars[var_idx].values = tmp;
                varstore.vars[var_idx].array_size = new_size;
                varstore.vars[var_idx].values[array_idx] =
                    eval_expr(assn->children[2], output_file);
                return;
            } else {
                fprintf(output_file, "warn: negative array offset\n");
                return;
            }
        }
    }

    // Variable not found - create new entry
    if (var_idx < VARSTORE_CAPACITY) {
        if (array_idx < 0) {
            fprintf(output_file, "warn: negative array offset\n");
            return;
        }

        varstore.vars[var_idx].beg = beg;
        varstore.vars[var_idx].len = len;
        varstore.vars[var_idx].values = malloc((array_idx + 1) * sizeof(int));
        varstore.vars[var_idx].array_size = 0;

        if (!varstore.vars[var_idx].values) {
            fprintf(output_file, "malloc failed\n");
            return;
        }

        varstore.vars[var_idx].array_size = array_idx + 1;
        varstore.vars[var_idx].values[array_idx] = eval_expr(assn->children[2], output_file);
        varstore.size++;
    } else {
        fprintf(output_file, "warn: varstore exhausted, assignment has no effect\n");
    }
}

// Execute a print statement
static void run_print(const struct node *const prnt, FILE *output_file)
{
    if (prnt->nchildren == 3) {
        // Simple print of expression
        fprintf(output_file, "%d\n", eval_expr(prnt->children[1], output_file));
    } else if (prnt->nchildren == 4) {
        // Print with string literal prefix
        const struct node *const strl = prnt->children[1];

        const uint8_t *const beg = strl->token->beg + 1;  // Skip opening quote
        const uint8_t *const end = strl->token->end - 1; // Skip closing quote
        const ptrdiff_t len = end - beg;

        fprintf(output_file, "%.*s%d\n", (int) len, beg, eval_expr(prnt->children[2], output_file));
    }
}

// Execute a control flow statement
static void run_ctrl(const struct node *const ctrl, FILE *output_file)
{
    switch (ctrl->children[0]->nt) {
    case NT_Cond: {  // If/elif/else statement
        const struct node *const cond = ctrl->children[0];

        if (eval_expr(cond->children[1], output_file)) {
            // Execute 'if' block
            const struct node *stmt = cond->children[3];
            while (stmt->nchildren) {
                run_statement(stmt++, output_file);
            }
        } else if (ctrl->nchildren >= 2) {
            // Check 'elif' and 'else' blocks
            size_t child_idx = 1;
            do {
                if (ctrl->children[child_idx]->nt == NT_Elif) {
                    const struct node *const elif = ctrl->children[child_idx];

                    if (eval_expr(elif->children[1], output_file)) {
                        // Execute 'elif' block
                        const struct node *stmt = elif->children[3];
                        while (stmt->nchildren) {
                            run_statement(stmt++, output_file);
                        }
                        break;
                    }
                } else {
                    // Execute 'else' block
                    const struct node *const els = ctrl->children[child_idx];
                    const struct node *stmt = els->children[2];
                    while (stmt->nchildren) {
                        run_statement(stmt++, output_file);
                    }
                }
            } while (++child_idx < ctrl->nchildren);
        }
    } break;

    case NT_Dowh: {  // Do-while loop
        const struct node *const dowh = ctrl->children[0];
        const struct node *const expr = dowh->children[dowh->nchildren - 2];

        do {
            // Execute loop body
            const struct node *stmt = dowh->children[2];
            while (stmt->nchildren) {
                run_statement(stmt++, output_file);
            }
        } while (eval_expr(expr, output_file));
    } break;

    case NT_Whil: {  // While loop
        const struct node *const whil = ctrl->children[0];

        while (eval_expr(whil->children[1], output_file)) {
            // Execute loop body
            const struct node *stmt = whil->children[3];
            while (stmt->nchildren) {
                run_statement(stmt++, output_file);
            }
        }
    } break;

    default:
        abort();  // Unknown control structure
    }
}

// Evaluate an atomic expression (variable or number)
static int eval_atom(const struct node *const atom, FILE *output_file)
{
    switch (atom->children[0]->token->token) {
    case token_NAME: {  // Variable reference
        const uint8_t *const beg = atom->children[0]->token->beg;
        const ptrdiff_t len = atom->children[0]->token->end - beg;

        // Look up variable in store
        for (size_t idx = 0; idx < varstore.size; ++idx) {
            if (varstore.vars[idx].len == len &&
                !memcmp(varstore.vars[idx].beg, beg, len)) {

                if (varstore.vars[idx].array_size) {
                    return varstore.vars[idx].values[0];  // Return scalar value
                } else {
                    return 0;  // Uninitialized array
                }
            }
        }

        fprintf(output_file, "warn: access to undefined variable\n");
        return 0;
    }

    case token_NMBR: {  // Numeric literal
        const uint8_t *const beg = atom->children[0]->token->beg;
        const uint8_t *const end = atom->children[0]->token->end;
        int result = 0, mult = 1;

        // Convert ASCII digits to integer
        for (ssize_t idx = end - beg - 1; idx >= 0; --idx, mult *= 10) {
            result += mult * (beg[idx] - '0');
        }

        return result;
    }

    default:
        abort();  // Unknown atom type
    }
}

// Evaluate an expression by delegating to appropriate sub-evaluator
static int eval_expr(const struct node *const expr, FILE *output_file)
{
    switch (expr->children[0]->nt) {
    case NT_Atom:  // Atomic expression
        return eval_atom(expr->children[0], output_file);

    case NT_Pexp:  // Parenthesized expression
        return eval_pexp(expr->children[0], output_file);

    case NT_Bexp:  // Binary operation
        return eval_bexp(expr->children[0], output_file);

    case NT_Uexp:  // Unary operation
        return eval_uexp(expr->children[0], output_file);

    case NT_Texp:  // Ternary conditional
        return eval_texp(expr->children[0], output_file);

    case NT_Aexp:  // Array access
        return eval_aexp(expr->children[0], output_file);

    default:
        abort();  // Unknown expression type
    }
}

// Evaluate a parenthesized expression
static int eval_pexp(const struct node *const pexp, FILE *output_file)
{
    // Just evaluate the expression inside the parentheses
    return eval_expr(pexp->children[1], output_file);
}

// Evaluate a binary operation
static int eval_bexp(const struct node *const bexp, FILE *output_file)
{
    // Evaluate both operands
    const int left = eval_expr(bexp->children[0], output_file);
    const int right = eval_expr(bexp->children[2], output_file);

    // Perform operation based on operator
    switch (bexp->children[1]->token->token) {
    case token_PLUS:  // Addition
        return left + right;

    case token_MINS:  // Subtraction
        return left - right;

    case token_MULT:  // Multiplication
        return left * right;

    case token_DIVI: {  // Division
        if (right) {
            return left / right;
        } else {
            fprintf(output_file, "warn: prevented attempt to divide by zero\n");
            return 0;
        }
    }

    case token_MODU:  // Modulo
        return left % right;

    case token_EQUL:  // Equality
        return left == right;

    case token_NEQL:  // Inequality
        return left != right;

    case token_LTHN:  // Less than
        return left < right;

    case token_GTHN:  // Greater than
        return left > right;

    case token_LTEQ:  // Less than or equal
        return left <= right;

    case token_GTEQ:  // Greater than or equal
        return left >= right;

    case token_CONJ:  // Logical AND
        return left && right;

    case token_DISJ:  // Logical OR
        return left || right;

    default:
        abort();  // Unknown operator
    }
}

// Evaluate a unary operation
static int eval_uexp(const struct node *const uexp, FILE *output_file)
{
    // Evaluate operand and apply unary operator
    switch (uexp->children[0]->token->token) {
    case token_PLUS:  // Unary plus
        return eval_expr(uexp->children[1], output_file);

    case token_MINS:  // Unary minus (negation)
        return -eval_expr(uexp->children[1], output_file);

    case token_NEGA:  // Logical negation
        return !eval_expr(uexp->children[1], output_file);

    default:
        abort();  // Unknown unary operator
    }
}

// Evaluate a ternary conditional expression
static int eval_texp(const struct node *const texp, FILE *output_file)
{
    // Evaluate condition and return appropriate branch
    return eval_expr(texp->children[0], output_file) ?
        eval_expr(texp->children[2], output_file) : eval_expr(texp->children[4], output_file);
}

// Evaluate an array access expression
static int eval_aexp(const struct node *const aexp, FILE *output_file)
{
    const uint8_t *const beg = aexp->children[0]->token->beg;
    const ptrdiff_t len = aexp->children[0]->token->end - beg;
    const int array_idx = eval_expr(aexp->children[2], output_file);

    if (array_idx < 0) {
        fprintf(output_file, "warn: negative array offset\n");
        return 0;
    }

    // Look up array in variable store
    for (size_t idx = 0; idx < varstore.size; ++idx) {
        if (varstore.vars[idx].len == len &&
            !memcmp(varstore.vars[idx].beg, beg, len)) {

            if (array_idx < varstore.vars[idx].array_size) {
                return varstore.vars[idx].values[array_idx];
            } else {
                fprintf(output_file, "warn: out of bounds array access\n");
                return 0;
            }
        }
    }

    fprintf(output_file, "warn: access to undefined array\n");
    return 0;
}