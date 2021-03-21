#include "owl/lexer.hpp"

#include <ctype.h>

namespace owl {

const char *token_name(token_t tok)
{
    // clang-format off
    static const char *names[TOKEN_SIZE] = {
            "<EOF>",
            "word",
            "number",
            "string",
            "'('",
            "')'",
            "'{'",
            "'}'",
            "'('",
            "')'",
            "','",
            "':'",
            "';'",
            "'='",
    };
    // clang-format on
    return names[tok];
}

void print_token(context *ctx, const token &t)
{
    fprintf(ctx->f_debug, "%s @%i, %i", token_name(t.tok), t.lnum, t.cnum);
    switch (t.tok) {
    case TOKEN_WORD:
    case TOKEN_NUMBER:
    case TOKEN_STRING:
        fprintf(ctx->f_debug, ": %.*s", (int) t.text.size(), t.text.data());
        break;
    default:
        break;
    }
    fprintf(ctx->f_debug, "\n");
}

bool tokenize(context *ctx, std::string_view code, std::vector<token> *tokens)
{
    int lnum = 1;
    size_t line_first = 0;
    for (size_t i = 0; i < code.size();) {
        int chr = code[i];
        if (isspace(chr)) {
            if (chr == '\n') {
                lnum++;
                line_first = i + 1;
            }
            i++;
            continue;
        }

        token t;

        const size_t first = i;
        if (chr == '_' || isalpha(chr)) {
            do {
                i++;
            } while (i < code.size() && (chr == '_' || isalnum(code[i])));

            t.text = code.substr(first, i - first);
            t.tok = TOKEN_WORD;
        } else if (isdigit(chr)) {
            do {
                i++;
            } while (i < code.size() && isdigit(code[i]));

            if (i < code.size() && isalpha(code[i])) {
                compiler_error_at(ctx,
                        lnum,
                        i - line_first + 1,
                        "invalid character: '%c' ord=%d",
                        (char) chr,
                        chr);
                return false;
            }

            t.text = code.substr(first, i - first);
            t.tok = TOKEN_NUMBER;
        } else {
            bool comment = false;

            switch (chr) {
            case '#':
                comment = true;
                break;

            case '(':
                t.tok = TOKEN_LPAREN;
                break;
            case ')':
                t.tok = TOKEN_RPAREN;
                break;

            case '{':
                t.tok = TOKEN_LCURLY;
                break;
            case '}':
                t.tok = TOKEN_RCURLY;
                break;

            case '[':
                t.tok = TOKEN_LINDEX;
                break;
            case ']':
                t.tok = TOKEN_RINDEX;
                break;

            case ',':
                t.tok = TOKEN_COMMA;
                break;
            case ':':
                t.tok = TOKEN_COLON;
                break;
            case ';':
                t.tok = TOKEN_SEMICOLON;
                break;
            case '=':
                t.tok = TOKEN_EQ;
                break;

            default:
                compiler_error_at(ctx,
                        lnum,
                        i - line_first + 1,
                        "invalid character: '%c' ord=%d",
                        (char) chr,
                        chr);
                return false;
            }

            if (comment) {
                while (i < code.size() && code[i] != '\n') {
                    i++;
                }
                continue;
            }

            i++;
        }

        if (ctx->debug_lexer) {
            print_token(ctx, t);
        }

        tokens->push_back(t);
    }

    token t_eof = {
            .tok = TOKEN_EOF,
            .lnum = lnum,
            .cnum = 1,
    };
    tokens->push_back(t_eof);

    return true;
}

bool is_keyword(std::string_view word)
{
    // TODO: Optimize by looking into first char

    // clang-format off
    return word == KW_AUTO
            || word == KW_DO
            || word == KW_IF
            || word == KW_FUNC
            || word == KW_OBJECT
            || word == KW_RETURN
            || word == KW_STRUCT
            || word == KW_VAR;
    // clang-format on
}

} // owl
