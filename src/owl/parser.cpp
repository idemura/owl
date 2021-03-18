#include "owl/parser.hpp"

namespace owl {

typedef struct {
    owl_context *parent_ctx;

    const owl_token *p_tokens;
    size_t n_tokens;

    size_t curr;
} owl_parse_ctx;

static void set_node(owl_node *node, owl_node_t type, const owl_token *t)
{
    node->type = type;
    node->lnum = t->lnum;
    node->cnum = t->cnum;
    node->text = t->text;
}

static void set_expr(owl_expr *node, owl_node_t type, const owl_token *t)
{
    set_node(&node->base, type, t);
    node->data_type.type_def = NULL;
}

static const owl_token *peek_token(owl_parse_ctx *ctx)
{
    return &ctx->p_tokens[ctx->curr];
}

static const owl_token *take_token(owl_parse_ctx *ctx)
{
    return &ctx->p_tokens[ctx->curr++];
}

static owl_data_type *owl_parse_data_type(owl_parse_ctx *ctx)
{
    const owl_token *t;

    if ((t = take_token(ctx))->tok != OWL_TOKEN_ID) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "data type: type name expected, found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    owl_data_type *e = memmgr_allocate_dirty(ctx->parent_ctx->mmc, sizeof(owl_data_type));
    set_node(&e->base, OWL_MN_EXPR, t);
    e->name = string_dup(t->text);

    return e;
}

static owl_expr *owl_parse_expr(owl_parse_ctx *ctx)
{
    const owl_token *t;

    if ((t = take_token(ctx))->tok != OWL_TOKEN_NUMBER) {
        owl_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "invalid expression",
                owl_token_name(t->tok));
        return NULL;
    }

    owl_expr_value *e = memmgr_allocate_dirty(ctx->parent_ctx->mmc, sizeof(owl_expr_value));
    set_expr(&e->expr, OWL_MN_EXPR, t);
    e->text = string_dup(t->text);

    return NULL;
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

    const owl_data_type *data_type = NULL;
    if ((t = peek_token(ctx))->tok == OWL_TOKEN_COLON) {
        ctx->curr++;
        data_type = owl_parse_data_type(ctx);
        if (!data_type) {
            return NULL;
        }
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
    set_node(&e->base, OWL_MN_FUNCTION, t_id);
    e->name = string_dup(t_id->text);

    printf("function: %.*s\n", (int) t_id->text.len, t_id->text.str);
    return e;
}

static owl_variable *owl_parse_variable(owl_parse_ctx *ctx)
{
    const owl_token *t;

    bool auto_var = false;
    if ((t = peek_token(ctx))->tok == OWL_KW_AUTO) {
        auto_var = true;
        ctx->curr++;
    }

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

    // Optional type
    owl_data_type *data_type = NULL;
    if ((t = peek_token(ctx))->tok == OWL_TOKEN_COLON) {
        ctx->curr++;
        data_type = owl_parse_data_type(ctx);
        if (!data_type) {
            return NULL;
        }
    }

    // Optional initializer
    owl_expr *init = NULL;
    if ((t = peek_token(ctx))->tok == OWL_TOKEN_EQ) {
        ctx->curr++;
        init = owl_parse_expr(ctx);
        if (!init) {
            return NULL;
        }
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
    set_node(&e->base, OWL_MN_VARIABLE, t_id);
    e->name = string_dup(t_id->text);
    e->data_type = data_type;
    e->init = init;
    e->auto_var = auto_var;

    printf("variable: %.*s\n", (int) t_id->text.len, t_id->text.str);
    return e;
}

static owl_object *owl_parse_object_def(owl_parse_ctx *ctx)
{
    const owl_token *t;

    if ((t = take_token(ctx))->tok != OWL_KW_OBJECT) {
        owl_error_at(ctx->parent_ctx, t->lnum, t->cnum, "object def expected");
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
                "object: expected '{', found %s",
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
                "object: expected '}', found %s",
                owl_token_name(t->tok));
        return NULL;
    }

    owl_object *e = memmgr_allocate_dirty(ctx->parent_ctx->mmc, sizeof(owl_object));
    set_node(&e->base, OWL_MN_OBJECT, t_id);
    e->name = string_dup(t_id->text);

    printf("object: %.*s\n", (int) t_id->text.len, t_id->text.str);
    return e;
}

static bool owl_parse_top_level_def(owl_parse_ctx *ctx, owl_unit *unit)
{
    const owl_token *t = peek_token(ctx);

    // Look ahead
    if (t->tok == OWL_KW_AUTO) {
        t = &ctx->p_tokens[ctx->curr + 1];
    }

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

    case OWL_KW_OBJECT: {
        owl_object *e = owl_parse_object_def(ctx);
        if (e) {
            vector_add(&unit->v_object, e);
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

    unit->base.type = OWL_MN_UNIT;
    unit->base.lnum = 0;
    unit->base.cnum = 0;
    unit->base.text = string_empty();

    vector_init(&unit->v_function);
    vector_init(&unit->v_variable);
    vector_init(&unit->v_object);
    vector_init(&unit->v_struct);

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

void owl_destroy_unit(owl_context *ctx, owl_unit *unit)
{
    if (!unit) {
        return;
    }

    vector_release(&unit->v_function);
    vector_release(&unit->v_variable);
    vector_release(&unit->v_object);
    vector_release(&unit->v_struct);

    memmgr_release(ctx->mmc, unit);
}

}
