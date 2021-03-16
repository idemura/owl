#include "owl/parser.h"

typedef struct {
    owl_context *parent_ctx;

    const owl_token *p_tokens;
    size_t n_tokens;

    size_t curr;
} owl_parse_ctx;

static void set_header(owl_node_header *head, owl_node_t type, const owl_token *t)
{
    head->type = type;
    head->lnum = t->lnum;
    head->cnum = t->cnum;
}

static const owl_token *peek_token(owl_parse_ctx *ctx)
{
    return &ctx->p_tokens[ctx->curr];
}

static const owl_token *take_token(owl_parse_ctx *ctx)
{
    return &ctx->p_tokens[ctx->curr++];
}

static owl_function *owl_parse_function(owl_parse_ctx *ctx)
{
    const owl_token *t;

    if ((t = take_token(ctx))->tok != OWL_KW_FUNC) {
        owl_error_at(ctx->parent_ctx, t->lnum, t->cnum, "function expected");
        return NULL;
    }

    if ((t = take_token(ctx))->tok != OWL_TOKEN_ID) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function name expected, found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    const owl_token *t_id = t;

    if ((t = take_token(ctx))->tok != OWL_TOKEN_LPAREN) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function argument list: expected '(', found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    // Arguments

    if ((t = take_token(ctx))->tok != OWL_TOKEN_RPAREN) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function argument list: expected ')', found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    const owl_token *t_type = NULL;
    if ((t = peek_token(ctx))->tok == OWL_TOKEN_COLON) {
        take_token(ctx);
        if ((t = take_token(ctx))->tok != OWL_TOKEN_ID) {
            owl_error_at(ctx->parent_ctx,
                    t->lnum,
                    t->cnum,
                    "function: return type expected, found %s",
                    owl_token_name(t->tok));
            return NULL;
        }
        t_type = t;
    }

    if ((t = take_token(ctx))->tok != OWL_TOKEN_LCURLY) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function: expected '{', found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    if ((t = take_token(ctx))->tok != OWL_TOKEN_RCURLY) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function: expected '}', found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    owl_function *e = memmgr_allocate_dirty(ctx->parent_ctx->mmc, sizeof(owl_function));
    set_header(&e->head, OWL_MN_FUNCTION, t_id);
    e->name = t_id->text;

    printf("func: %.*s\n", (int) t_id->text.len, t_id->text.str);
    return e;
}

static owl_variable *owl_parse_variable(owl_parse_ctx *ctx)
{
    const owl_token *t;

    if ((t = take_token(ctx))->tok != OWL_KW_VAR) {
        owl_error_at(ctx->parent_ctx, t->lnum, t->cnum, "variable expected");
        return NULL;
    }

    if ((t = take_token(ctx))->tok != OWL_TOKEN_ID) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "variable name expected, found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    const owl_token *t_id = t;

    const owl_token *t_type = NULL;
    if ((t = peek_token(ctx))->tok == OWL_TOKEN_COLON) {
        take_token(ctx);
        // Optional type
        if ((t = take_token(ctx))->tok != OWL_TOKEN_ID) {
            owl_error_at(ctx->parent_ctx,
                    t->lnum,
                    t->cnum,
                    "variable: type expected, found %s",
                    owl_token_name(t->tok));
            return NULL;
        }
        t_type = t;
    }

    const owl_token *t_init = NULL;
    if ((t = peek_token(ctx))->tok == OWL_TOKEN_EQ) {
        take_token(ctx);
        // Optional initializer
        if ((t = take_token(ctx))->tok != OWL_TOKEN_NUMBER) {
            owl_error_at(
                    ctx->parent_ctx, t->lnum, t->cnum, "variable: initializer expression expected");
            return NULL;
        }
        t_init = t;
    }

    if ((t = take_token(ctx))->tok != OWL_TOKEN_SEMICOLON) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "variable: ';' expected, found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    owl_variable *e = memmgr_allocate_dirty(ctx->parent_ctx->mmc, sizeof(owl_variable));
    set_header(&e->head, OWL_MN_VARIABLE, t_id);
    e->name = t_id->text;

    printf("var: %.*s\n", (int) t_id->text.len, t_id->text.str);
    return e;
}

static owl_class *owl_parse_class_type(owl_parse_ctx *ctx)
{
    const owl_token *t;

    if ((t = take_token(ctx))->tok != OWL_KW_CLASS) {
        owl_error_at(ctx->parent_ctx, t->lnum, t->cnum, "'class' expected");
        return NULL;
    }

    if ((t = take_token(ctx))->tok != OWL_TOKEN_ID) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "type name expected, found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    const owl_token *t_id = t;

    if ((t = take_token(ctx))->tok != OWL_TOKEN_LCURLY) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "class type: expected '{', found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    while (peek_token(ctx)->tok != OWL_TOKEN_RCURLY) {
        owl_variable *field = owl_parse_variable(ctx);
        if (!field) {
            return NULL;
        }
        printf("field: %.*s\n", (int) field->name.len, field->name.str);
    }

    if ((t = take_token(ctx))->tok != OWL_TOKEN_RCURLY) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "class type: expected '}', found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    owl_class *e = memmgr_allocate_dirty(ctx->parent_ctx->mmc, sizeof(owl_class));
    set_header(&e->head, OWL_MN_CLASS, t_id);
    e->name = t_id->text;

    printf("class: %.*s\n", (int) t_id->text.len, t_id->text.str);
    return e;
}

static bool owl_parse_top_level_def(owl_parse_ctx *ctx, owl_unit *unit)
{
    const owl_token *t = peek_token(ctx);
    switch (t->tok) {
    case OWL_TOKEN_EOF:
        return true;

    case OWL_KW_FUNC: {
        owl_function *e = owl_parse_function(ctx);
        if (e) {
            vector_add(&unit->v_function, e);
        }
        return e != NULL;
    }

    case OWL_KW_VAR: {
        owl_variable *e = owl_parse_variable(ctx);
        if (e) {
            vector_add(&unit->v_variable, e);
        }
        return e != NULL;
    }

    case OWL_KW_CLASS: {
        owl_class *e = owl_parse_class_type(ctx);
        if (e) {
            vector_add(&unit->v_class, e);
        }
        return e != NULL;
    }

    default:
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "unrecognized top level construct starting with %s",
                owl_token_name(t->tok));
        return false;
    }
}

static owl_unit *owl_parse_unit(owl_parse_ctx *ctx)
{
    owl_unit *unit = memmgr_allocate_dirty(ctx->parent_ctx->mmc, sizeof(owl_unit));
    vector_init(&unit->v_function);
    vector_init(&unit->v_variable);
    vector_init(&unit->v_class);
    vector_init(&unit->v_value);

    const owl_token *t = peek_token(ctx);
    while (t->tok != OWL_TOKEN_EOF && owl_parse_top_level_def(ctx, unit)) {
        t = peek_token(ctx);
    }

    return unit;
}

owl_unit *owl_parse(owl_context *ctx, const owl_token *p_tokens, size_t n_tokens)
{
    owl_parse_ctx parse_ctx = {};
    parse_ctx.parent_ctx = ctx;
    parse_ctx.p_tokens = p_tokens;
    parse_ctx.n_tokens = n_tokens;
    parse_ctx.curr = 0;

    owl_unit *unit = owl_parse_unit(&parse_ctx);

    return unit;
}
