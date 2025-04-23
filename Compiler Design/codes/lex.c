#include "lex.h"
#include <stdio.h>
#include <stdlib.h>

// Define the possible states for the state machine
enum {
    STS_ACCEPT,  // Token is fully recognized
    STS_REJECT,  // Token is not recognized
    STS_HUNGRY,  // Token is partially recognized, more input is needed
};

typedef uint8_t sts_t;  // State type, used to store the current state

// Macro to transition between states
#define TR(st, tr) (*s = (st), (STS_##tr))
#define REJECT TR(0, REJECT)  // Macro to reject a token

// Macros to check character types
#define IS_ALPHA(c)  (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define IS_DIGIT(c)  ((c) >= '0' && (c) <= '9')
#define IS_ALNUM(c)  (IS_ALPHA(c) || IS_DIGIT(c))
#define IS_WHITESPACE(c) ((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n')

// Macros to define token recognition functions for tokens of different lengths
#define TOKEN_DEFINE_1(token, str) \
static sts_t token(const uint8_t c, uint8_t *const s) \
{ \
    switch (*s) { \
    case 0: return c == (str)[0] ? TR(1, ACCEPT) : REJECT; \
    case 1: return REJECT; \
    default: abort(); \
    } \
}

#define TOKEN_DEFINE_2(token, str) \
static sts_t token(const uint8_t c, uint8_t *const s) \
{ \
    switch (*s) { \
    case 0: return c == (str)[0] ? TR(1, HUNGRY) : REJECT; \
    case 1: return c == (str)[1] ? TR(2, ACCEPT) : REJECT; \
    case 2: return REJECT; \
    default: abort(); \
    } \
}

#define TOKEN_DEFINE_3(token, str) \
static sts_t token(const uint8_t c, uint8_t *const s) \
{ \
    switch (*s) { \
    case 0: return c == (str)[0] ? TR(1, HUNGRY) : REJECT; \
    case 1: return c == (str)[1] ? TR(2, HUNGRY) : REJECT; \
    case 2: return c == (str)[2] ? TR(3, ACCEPT) : REJECT; \
    case 3: return REJECT; \
    default: abort(); \
    } \
}

#define TOKEN_DEFINE_4(token, str) \
static sts_t token(const uint8_t c, uint8_t *const s) \
{ \
    switch (*s) { \
    case 0: return c == (str)[0] ? TR(1, HUNGRY) : REJECT; \
    case 1: return c == (str)[1] ? TR(2, HUNGRY) : REJECT; \
    case 2: return c == (str)[2] ? TR(3, HUNGRY) : REJECT; \
    case 3: return c == (str)[3] ? TR(4, ACCEPT) : REJECT; \
    case 4: return REJECT; \
    default: abort(); \
    } \
}

#define TOKEN_DEFINE_5(token, str) \
static sts_t token(const uint8_t c, uint8_t *const s) \
{ \
    switch (*s) { \
    case 0: return c == (str)[0] ? TR(1, HUNGRY) : REJECT; \
    case 1: return c == (str)[1] ? TR(2, HUNGRY) : REJECT; \
    case 2: return c == (str)[2] ? TR(3, HUNGRY) : REJECT; \
    case 3: return c == (str)[3] ? TR(4, HUNGRY) : REJECT; \
    case 4: return c == (str)[4] ? TR(5, ACCEPT) : REJECT; \
    case 5: return REJECT; \
    default: abort(); \
    } \
}

// Token recognition functions for specific token types
static sts_t token_name(const uint8_t c, uint8_t *const s)
{
    enum {
        token_name_beginin,
        token_name_accum,
    };

    switch (*s) {
    case token_name_beginin:
        return IS_ALPHA(c) || (c == '_') ? TR(token_name_accum, ACCEPT) : REJECT;

    case token_name_accum:
        return IS_ALNUM(c) || (c == '_') ? STS_ACCEPT : REJECT;
    }

    abort();
}

static sts_t token_nmbr(const uint8_t c, uint8_t *const s)
{
    (void) s;
    return IS_DIGIT(c) ? STS_ACCEPT : STS_REJECT;
}

static sts_t token_strl(const uint8_t c, uint8_t *const s)
{
    enum {
        token_strl_begin,
        token_strl_accum,
        token_strl_end,
    };

    switch (*s) {
    case token_strl_begin:
        return c == '"' ? TR(token_strl_accum, HUNGRY) : REJECT;

    case token_strl_accum:
        return c != '"' ? STS_HUNGRY : TR(token_strl_end, ACCEPT);

    case token_strl_end:
        return REJECT;
    }

    abort();
}

static sts_t token_wspc(const uint8_t c, uint8_t *const s)
{
    enum {
        token_wspc_begin,
        token_wspc_accum,
    };

    switch (*s) {
    case token_wspc_begin:
        return IS_WHITESPACE(c) ? TR(token_wspc_accum, ACCEPT) : REJECT;

    case token_wspc_accum:
        return IS_WHITESPACE(c) ? STS_ACCEPT : REJECT;
    }

    abort();
}

static sts_t token_lcom(const uint8_t c, uint8_t *const s)
{
    enum {
        token_lcom_begin,
        token_lcom_first_slash,
        token_lcom_accum,
        token_lcom_end
    };

    switch (*s) {
    case token_lcom_begin:
        return c == '/' ? TR(token_lcom_first_slash, HUNGRY) : REJECT;

    case token_lcom_first_slash:
        return c == '/' ? TR(token_lcom_accum, HUNGRY) : REJECT;

    case token_lcom_accum:
        return c == '\n' || c == '\r' ? TR(token_lcom_end, ACCEPT) : STS_HUNGRY;

    case token_lcom_end:
        return REJECT;
    }

    abort();
}

static sts_t token_bcom(const uint8_t c, uint8_t *const s)
{
    enum {
        token_bcom_begin,
        token_bcom_open_slash,
        token_bcom_accum,
        token_bcom_close_star,
        token_bcom_end
    };

    switch (*s) {
    case token_bcom_begin:
        return c == '/' ? TR(token_bcom_open_slash, HUNGRY) : REJECT;

    case token_bcom_open_slash:
        return c == '*' ? TR(token_bcom_accum, HUNGRY) : REJECT;

    case token_bcom_accum:
        return c != '*' ? STS_HUNGRY : TR(token_bcom_close_star, HUNGRY);

    case token_bcom_close_star:
        return c == '/' ? TR(token_bcom_end, ACCEPT) : TR(token_bcom_accum, HUNGRY);

    case token_bcom_end:
        return REJECT;
    }

    abort();
}

// Define token recognition functions for specific tokens
TOKEN_DEFINE_1(token_lpar, "(")
TOKEN_DEFINE_1(token_rpar, ")")
TOKEN_DEFINE_1(token_lbra, "[")
TOKEN_DEFINE_1(token_rbra, "]")
TOKEN_DEFINE_1(token_lbrc, "{")
TOKEN_DEFINE_1(token_rbrc, "}")
TOKEN_DEFINE_2(token_cond, "if")
TOKEN_DEFINE_4(token_elif, "elif")
TOKEN_DEFINE_4(token_else, "else")
TOKEN_DEFINE_2(token_dowh, "do")
TOKEN_DEFINE_5(token_whil, "while")
TOKEN_DEFINE_1(token_assn, "=")
TOKEN_DEFINE_2(token_equl, "==")
TOKEN_DEFINE_2(token_neql, "!=")
TOKEN_DEFINE_1(token_lthn, "<")
TOKEN_DEFINE_1(token_gthn, ">")
TOKEN_DEFINE_2(token_lteq, "<=")
TOKEN_DEFINE_2(token_gteq, ">=")
TOKEN_DEFINE_2(token_conj, "&&")
TOKEN_DEFINE_2(token_disj, "||")
TOKEN_DEFINE_1(token_plus, "+")
TOKEN_DEFINE_1(token_mins, "-")
TOKEN_DEFINE_1(token_mult, "*")
TOKEN_DEFINE_1(token_divi, "/")
TOKEN_DEFINE_1(token_modu, "%")
TOKEN_DEFINE_1(token_nega, "!")
TOKEN_DEFINE_5(token_prnt, "print")
TOKEN_DEFINE_1(token_scol, ";")
TOKEN_DEFINE_1(token_ques, "?")
TOKEN_DEFINE_1(token_coln, ":")

// Array of token recognition functions
static sts_t (*const token_funcs[token_COUNT])(const uint8_t, uint8_t *const) = {
    token_name,
    token_nmbr,
    token_strl,
    token_wspc,
    token_lcom,
    token_bcom,
    token_lpar,
    token_rpar,
    token_lbra,
    token_rbra,
    token_lbrc,
    token_rbrc,
    token_cond,
    token_elif,
    token_else,
    token_dowh,
    token_whil,
    token_assn,
    token_equl,
    token_neql,
    token_lthn,
    token_gthn,
    token_lteq,
    token_gteq,
    token_conj,
    token_disj,
    token_plus,
    token_mins,
    token_mult,
    token_divi,
    token_modu,
    token_nega,
    token_prnt,
    token_scol,
    token_ques,
    token_coln,
};

// Function to push a recognized token into the token list
static inline int push_token(struct token **const tokens,
    size_t *const ntokens, size_t *const allocated, const token_t token,
    const uint8_t *const beg, const uint8_t *const end)
{
    if (*ntokens >= *allocated) {
        *allocated = (*allocated ?: 1) * 8;

        struct token *const tmp =
            realloc(*tokens, *allocated * sizeof(struct token));

        if (!tmp) {
            return free(*tokens), *tokens = NULL, LEX_NOMEM;
        }

        *tokens = tmp;
    }

    (*tokens)[(*ntokens)++] = (struct token) {
        .beg = beg,
        .end = end,
        .token = token
    };

    return LEX_OK;
}

// Main lexer function
int lex(const uint8_t *const input, const size_t size,
    struct token **const tokens, size_t *const ntokens)
{
    static struct {
        sts_t prev, curr;
    } statuses[token_COUNT] = {
        [0 ... token_COUNT - 1] = { STS_HUNGRY, STS_REJECT }
    };

    uint8_t states[token_COUNT] = {0};

    const uint8_t *prefix_begin = input, *prefix_end = input;
    token_t accepted_token;
    size_t allocated = 0;
    *tokens = NULL, *ntokens = 0;

    #define PUSH_OR_NOMEM(token, beg, end) \
        if (push_token(tokens, ntokens, &allocated, (token), (beg), (end))) { \
            return LEX_NOMEM; \
        }

    #define foreach_token \
        for (token_t token = 0; token < token_COUNT; ++token)

    PUSH_OR_NOMEM(token_FBEG, NULL, NULL);

    while (prefix_end < input + size) {
        int did_accept = 0;

        foreach_token {
            if (statuses[token].prev != STS_REJECT) {
                statuses[token].curr = token_funcs[token](*prefix_end, &states[token]);
            }

            if (statuses[token].curr != STS_REJECT) {
                did_accept = 1;
            }
        }

        if (did_accept) {
            prefix_end++;

            foreach_token {
                statuses[token].prev = statuses[token].curr;
            }
        } else {
            accepted_token = token_COUNT;

            foreach_token {
                if (statuses[token].prev == STS_ACCEPT) {
                    accepted_token = token;
                }

                statuses[token].prev = STS_HUNGRY;
                statuses[token].curr = STS_REJECT;
            }

            PUSH_OR_NOMEM(accepted_token, prefix_begin, prefix_end);

            if (accepted_token == token_COUNT) {
                (*tokens)[*ntokens - 1].end++;
                return LEX_UNKNOWN_TOKEN;
            }

            prefix_begin = prefix_end;
        }
    }

    accepted_token = token_COUNT;

    foreach_token {
        if (statuses[token].curr == STS_ACCEPT) {
            accepted_token = token;
        }

        statuses[token].prev = STS_HUNGRY;
        statuses[token].curr = STS_REJECT;
    }

    PUSH_OR_NOMEM(accepted_token, prefix_begin, prefix_end);

    if (accepted_token == token_COUNT) {
        return LEX_UNKNOWN_TOKEN;
    }

    PUSH_OR_NOMEM(token_FEND, NULL, NULL);
    return LEX_OK;

    #undef PUSH_OR_NOMEM
    #undef foreach_token
}