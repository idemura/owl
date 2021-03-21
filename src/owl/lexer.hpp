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

#define KW_AUTO "auto"
#define KW_DO "do"
#define KW_IF "if"
#define KW_FUNC "func"
#define KW_OBJECT "object"
#define KW_RETURN "return"
#define KW_STRUCT "struct"
#define KW_VAR "var"

enum token_t {
    TOKEN_EOF, // end-of-file

    TOKEN_WORD,

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
bool is_keyword(std::string_view word);

} // owl

#endif
