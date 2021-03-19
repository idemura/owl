#ifndef LEXER_HPP
#define LEXER_HPP

#include "owl/context.hpp"

#include <string>
#include <string_view>
#include <vector>

/**
 * Lexer. Converts text into a vector of tokens.
 */

namespace owl {

enum token_t {
    TOKEN_EOF, // end-of-file

    TOKEN_ID,

    KW_AUTO,
    KW_DO,
    KW_IF,
    KW_FUNC,
    KW_OBJECT,
    KW_RETURN,
    KW_STRUCT,
    KW_VAR,

    TOKEN_NUMBER,
    TOKEN_STRING,

    TOKEN_LPAREN, // (
    TOKEN_RPAREN, // )

    TOKEN_LCURLY, // {
    TOKEN_RCURLY, // }

    TOKEN_LINDEX, // [
    TOKEN_RINDEX, // ]

    TOKEN_COMMA, // ,
    TOKEN_COLON, // :
    TOKEN_SEMICOLON, // ;
    TOKEN_EQ, // =

    TOKEN_SIZE
};

struct token {
    token_t tok = TOKEN_EOF;
    int lnum = 0;
    int cnum = 0;
    std::string_view text;
};

bool tokenize(context *ctx, std::string_view code, std::vector<token> *tokens);
const char *token_name(token_t tok);
void print_token(context *ctx, const token &t);

} // owl

#endif
