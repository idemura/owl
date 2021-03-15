#include "owl/lexer.h"

#include <ctype.h>

static owl_token_t owl_translate_word(string w)
{
    switch (w.str[0]) {
    case 'd':
        if (string_eq_sz(w, "do")) {
            return OWL_KW_DO;
        }
        break;
    case 'i':
        if (string_eq_sz(w, "if")) {
            return OWL_KW_IF;
        }
        break;
    case 'f':
        if (string_eq_sz(w, "fn")) {
            return OWL_KW_FN;
        }
        break;
    case 'r':
        if (string_eq_sz(w, "return")) {
            return OWL_KW_RETURN;
        }
        break;
    case 'v':
        if (string_eq_sz(w, "var")) {
            return OWL_KW_VAR;
        }
        break;
    }
    return OWL_TOKEN_ID;
}

const char *owl_token_name(owl_token_t tok)
{
    // clang-format off
    static const char *names[OWL_TOKEN_SIZE] = {
            "<EOF>",
            "id",
            "do",
            "if",
            "fn",
            "return",
            "var",
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
    };
    // clang-format on
    return names[tok];
}

void owl_print_token(owl_context *ctx, const owl_token *t)
{
    fprintf(ctx->f_debug, "%s @%i, %i", owl_token_name(t->tok), t->lnum, t->cnum);
    switch (t->tok) {
    case OWL_TOKEN_ID:
    case OWL_TOKEN_NUMBER:
    case OWL_TOKEN_STRING:
        fprintf(ctx->f_debug, ": %.*s", (int) t->text.len, t->text.str);
        break;
    default:
        break;
    }
    fprintf(ctx->f_debug, "\n");
}

bool owl_tokenize(owl_context *ctx, string code, vector_owl_token *tokens)
{
    int lnum = 1;
    size_t line_first = 0;
    for (size_t i = 0; i < code.len;) {
        int chr = code.str[i];
        if (isspace(chr)) {
            if (chr == '\n') {
                lnum++;
                line_first = i + 1;
            }
            i++;
            continue;
        }

        owl_token t = {
                .tok = OWL_TOKEN_EOF,
                .lnum = lnum,
                .cnum = (int) (i - line_first + 1),
                .text = string_empty(),
        };

        const size_t first = i;
        if (chr == '_' || isalpha(chr)) {
            do {
                i++;
            } while (i < code.len && (chr == '_' || isalnum(code.str[i])));

            t.text = string_of_len(code.str + first, i - first);
            t.tok = owl_translate_word(t.text);
        } else if (isdigit(chr)) {
            do {
                i++;
            } while (i < code.len && isdigit(code.str[i]));

            if (i < code.len && isalpha(code.str[i])) {
                owl_error_at(ctx, lnum, line_first - i + 1, "invalid character: ord=%d", chr);
                return false;
            }

            t.text = string_of_len(code.str + first, i - first);
            t.tok = OWL_TOKEN_NUMBER;
        } else {
            bool comment = false;

            switch (chr) {
            case '#':
                comment = true;
                break;

            case '(':
                t.tok = OWL_TOKEN_LPAREN;
                break;
            case ')':
                t.tok = OWL_TOKEN_RPAREN;
                break;

            case '{':
                t.tok = OWL_TOKEN_LCURLY;
                break;
            case '}':
                t.tok = OWL_TOKEN_RCURLY;
                break;

            case '[':
                t.tok = OWL_TOKEN_LINDEX;
                break;
            case ']':
                t.tok = OWL_TOKEN_RINDEX;
                break;

            case ',':
                t.tok = OWL_TOKEN_COMMA;
                break;
            case ':':
                t.tok = OWL_TOKEN_COLON;
                break;
            case ';':
                t.tok = OWL_TOKEN_SEMICOLON;
                break;

            default:
                owl_error_at(ctx, lnum, line_first - i + 1, "invalid character: ord=%d", chr);
                return false;
            }

            if (comment) {
                while (i < code.len && code.str[i] != '\n') {
                    i++;
                }
                continue;
            }

            i++;
        }

        if (ctx->debug_lexer) {
            owl_print_token(ctx, &t);
        }

        vector_add(tokens, t);
    }

    owl_token t_eof = {
            .tok = OWL_TOKEN_EOF,
            .lnum = lnum,
            .cnum = 1,
            .text = string_empty(),
    };
    vector_add(tokens, t_eof);

    return true;
}
