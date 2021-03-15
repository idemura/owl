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

static const owl_token *take_token(owl_parse_ctx *ctx)
{
    return &ctx->p_tokens[ctx->curr++];
}

static owl_function *owl_parse_function(owl_parse_ctx *ctx)
{
    const owl_token *t = take_token(ctx);
    if (t->tok != OWL_KW_FUNC) {
        owl_error_at(ctx->parent_ctx, t->lnum, t->cnum, "'func' expected");
        return NULL;
    }

    t = take_token(ctx);
    if (t->tok != OWL_TOKEN_ID) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function name expected, found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    const owl_token *t_id = t;

    t = take_token(ctx);
    if (t->tok != OWL_TOKEN_LPAREN) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function argument list: expected '(', found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    t = take_token(ctx);
    if (t->tok != OWL_TOKEN_RPAREN) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function argument list: expected ')', found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    const owl_token *t_type = NULL;
    t = take_token(ctx);
    if (t->tok == OWL_TOKEN_COLON) {
        t = take_token(ctx);
        if (t->tok != OWL_TOKEN_ID) {
            owl_error_at(ctx->parent_ctx,
                    t->lnum,
                    t->cnum,
                    "function: return type expected, found %s",
                    owl_token_name(t->tok));
            return NULL;
        }
        t_type = t;
    }

    t = take_token(ctx);
    if (t->tok != OWL_TOKEN_LCURLY) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function: expected '{', found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    t = take_token(ctx);
    if (t->tok != OWL_TOKEN_RCURLY) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function: expected '}', found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    owl_function *e = memmgr_allocate_dirty(ctx->parent_ctx->mmc, sizeof(owl_function));
    e->type = OWL_MN_FUNCTION;
    e->name = t_id->text;
    printf("func: %.*s\n", (int) t_id->text.len, t_id->text.str);
    return e;
}

static owl_variable *owl_parse_variable(owl_parse_ctx *ctx)
{
    const owl_token *t = take_token(ctx);
    if (t->tok != OWL_KW_VAR) {
        owl_error_at(ctx->parent_ctx, t->lnum, t->cnum, "'var' expected");
        return NULL;
    }

    t = take_token(ctx);
    if (t->tok != OWL_TOKEN_ID) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "variable name expected, found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    const owl_token *t_id = t;

    t = take_token(ctx);
    if (t->tok != OWL_TOKEN_EQ) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "variable: '=' after name expected, found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    t = take_token(ctx);
    if (t->tok != OWL_TOKEN_NUMBER) {
        owl_error_at(ctx->parent_ctx, t->lnum, t->cnum, "variable: expression expected");
        return NULL;
    }

    t = take_token(ctx);
    if (t->tok != OWL_TOKEN_SEMICOLON) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "variable: ';' expected, found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    owl_variable *e = memmgr_allocate_dirty(ctx->parent_ctx->mmc, sizeof(owl_variable));
    e->type = OWL_MN_VARIABLE;
    e->name = t_id->text;
    printf("var: %.*s\n", (int) t_id->text.len, t_id->text.str);
    return e;
}

static owl_class *owl_parse_class_type(owl_parse_ctx *ctx)
{
    const owl_token *t = take_token(ctx);
    if (t->tok != OWL_KW_CLASS) {
        owl_error_at(ctx->parent_ctx, t->lnum, t->cnum, "'class' expected");
        return NULL;
    }

    t = take_token(ctx);
    if (t->tok != OWL_TOKEN_ID) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "class name expected, found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    const owl_token *t_id = t;

    t = take_token(ctx);
    if (t->tok != OWL_TOKEN_LCURLY) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "class: expected '{', found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    t = peek_token(ctx);
    while (t->tok != OWL_TOKEN_RCURLY) {
        owl_variable *field = owl_parse_variable(ctx);
        printf("field: %.*s\n", (int) field->name.len, field->name.str);
        t = peek_token(ctx);
    }

    t = take_token(ctx);
    if (t->tok != OWL_TOKEN_RCURLY) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "class: expected '}', found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    owl_class *e = memmgr_allocate_dirty(ctx->parent_ctx->mmc, sizeof(owl_class));
    e->type = OWL_MN_CLASS;
    e->name = t_id->text;
    printf("class: %.*s\n", (int) t_id->text.len, t_id->text.str);
    return e;
}

static owl_value *owl_parse_value_type(owl_parse_ctx *ctx)
{
    const owl_token *t = take_token(ctx);
    if (t->tok != OWL_KW_CLASS) {
        owl_error_at(ctx->parent_ctx, t->lnum, t->cnum, "'value' expected");
        return NULL;
    }

    t = take_token(ctx);
    if (t->tok != OWL_TOKEN_ID) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "value name expected, found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    const owl_token *t_id = t;

    t = take_token(ctx);
    if (t->tok != OWL_TOKEN_LCURLY) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "value: expected '{', found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    t = peek_token(ctx);
    while (t->tok != OWL_TOKEN_RCURLY) {
        //
    }

    t = take_token(ctx);
    if (t->tok != OWL_TOKEN_RCURLY) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "value: expected '}', found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    owl_value *e = memmgr_allocate_dirty(ctx->parent_ctx->mmc, sizeof(owl_value));
    e->type = OWL_MN_CLASS;
    e->name = t_id->text;
    printf("value: %.*s\n", (int) t_id->text.len, t_id->text.str);
    return e;
}

static bool owl_parse_top_level_def(owl_parse_ctx *ctx, owl_unit *unit)
{
    const owl_token *t = peek_token(ctx);
    switch (t->tok) {
    case OWL_TOKEN_EOF:
        return true;

    case OWL_KW_FUNC:
        vector_add(&unit->v_function, owl_parse_function(ctx));
        return true;

    case OWL_KW_VAR:
        vector_add(&unit->v_variable, owl_parse_variable(ctx));
        return true;

    case OWL_KW_CLASS:
        vector_add(&unit->v_class, owl_parse_class_type(ctx));
        return true;

    case OWL_KW_VALUE:
        vector_add(&unit->v_value, owl_parse_value_type(ctx));
        return true;

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

bool owl_parse(owl_context *ctx, const owl_token *p_tokens, size_t n_tokens)
{
    owl_parse_ctx parse_ctx = {};
    parse_ctx.parent_ctx = ctx;
    parse_ctx.p_tokens = p_tokens;
    parse_ctx.n_tokens = n_tokens;
    parse_ctx.curr = 0;

    owl_unit *unit = owl_parse_unit(&parse_ctx);
    memmgr_release(ctx->mmc, unit);

    return true;
}
