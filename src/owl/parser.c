#include "owl/parser.h"

typedef struct {
    owl_context *parent_ctx;

    const owl_token *p_tokens;
    size_t n_tokens;

    size_t curr;
} owl_parse_ctx;

static const owl_token *peek_token(owl_parse_ctx *ctx)
{
    return &ctx->p_tokens[ctx->curr];
}

static const owl_token *next_token(owl_parse_ctx *ctx)
{
    ctx->curr++;
    return peek_token(ctx);
}

static bool owl_parse_function(owl_parse_ctx *ctx, owl_unit *unit)
{
    const owl_token *t = peek_token(ctx);
    if (t->tok != OWL_KW_FN) {
        owl_error_at(ctx->parent_ctx, t->lnum, t->cnum, "function expected");
        return false;
    }

    t = next_token(ctx);
    if (t->tok != OWL_TOKEN_ID) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function name expected, found %s",
                owl_token_name(t->tok));
        return false;
    }

    const owl_token *t_id = t;

    t = next_token(ctx);
    if (t->tok != OWL_TOKEN_LPAREN) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function argument list: expected '(', found %s",
                owl_token_name(t->tok));
        return false;
    }

    t = next_token(ctx);
    if (t->tok != OWL_TOKEN_RPAREN) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function argument list: expected ')', found %s",
                owl_token_name(t->tok));
        return false;
    }

    const owl_token *t_type = NULL;
    t = next_token(ctx);
    if (t->tok == OWL_TOKEN_COLON) {
        t = next_token(ctx);
        if (t->tok != OWL_TOKEN_ID) {
            owl_error_at(ctx->parent_ctx,
                    t->lnum,
                    t->cnum,
                    "function: return type expected, found %s",
                    owl_token_name(t->tok));
            return false;
        }
        t_type = t;
    }

    t = next_token(ctx);
    if (t->tok != OWL_TOKEN_LCURLY) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function: expected '{', found %s",
                owl_token_name(t->tok));
        return false;
    }

    t = next_token(ctx);
    if (t->tok != OWL_TOKEN_RCURLY) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function: expected '}', found %s",
                owl_token_name(t->tok));
        return false;
    }

    t = next_token(ctx);
    printf("func: %.*s\n", (int) t_id->text.len, t_id->text.str);

    return true;
}

static owl_unit *owl_parse_unit(owl_parse_ctx *ctx)
{
    owl_unit *unit = memmgr_allocate_dirty(MMC(ctx->parent_ctx), sizeof(owl_unit));

    const owl_token *t = peek_token(ctx);
    while (t->tok != OWL_TOKEN_EOF && owl_parse_function(ctx, unit)) {
        t = peek_token(ctx);
        printf("%s\n", owl_token_name(t->tok));
    }

    return unit;
}

bool owl_parse(owl_context *ctx, const owl_token *p_tokens, size_t n_tokens)
{
    owl_parse_ctx parse_ctx = {};
    parse_ctx.parent_ctx = ctx;
    parse_ctx.p_tokens = p_tokens;
    parse_ctx.n_tokens = n_tokens;
    parse_ctx.curr = 0;

    owl_unit *unit = owl_parse_unit(&parse_ctx);
    memmgr_release(MMC(ctx), unit);

    return true;
}
