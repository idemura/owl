#ifndef OWL_LEXER_H
#define OWL_LEXER_H

#include "foundation/string.h"
#include "foundation/vector.h"
#include "owl/context.h"

/**
 * Lexer. Converts text into a vector of tokens.
 */

typedef enum {
    OWL_TOKEN_EOF, // end-of-file

    OWL_TOKEN_ID,

    OWL_KW_AUTO,
    OWL_KW_CLASS,
    OWL_KW_DO,
    OWL_KW_IF,
    OWL_KW_FUNC,
    OWL_KW_RETURN,
    OWL_KW_VALUE,
    OWL_KW_VAR,

    OWL_TOKEN_NUMBER,
    OWL_TOKEN_STRING,

    OWL_TOKEN_LPAREN, // (
    OWL_TOKEN_RPAREN, // )

    OWL_TOKEN_LCURLY, // {
    OWL_TOKEN_RCURLY, // }

    OWL_TOKEN_LINDEX, // [
    OWL_TOKEN_RINDEX, // ]

    OWL_TOKEN_COMMA, // ,
    OWL_TOKEN_COLON, // :
    OWL_TOKEN_SEMICOLON, // ;
    OWL_TOKEN_EQ, // =

    OWL_TOKEN_SIZE
} owl_token_t;

typedef struct {
    owl_token_t tok;
    int lnum;
    int cnum;
    string text;
} owl_token;

typedef def_vector(owl_token) vector_owl_token;

bool owl_tokenize(owl_context *ctx, string code, vector_owl_token *tokens);
const char *owl_token_name(owl_token_t tok);
void owl_print_token(owl_context *ctx, const owl_token *t);

#endif
