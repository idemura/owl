#include "owl/parser.hpp"

namespace owl {

struct parse_ctx {
    context *parent_ctx = nullptr;

    const token *p_tokens = nullptr;
    size_t n_tokens = 0;

    size_t curr = 0;
};

static void set_node(mod_node *node, const token *t)
{
    node->lnum = t->lnum;
    node->cnum = t->cnum;
    node->text = t->text;
}

static const token *peek_token(parse_ctx *ctx)
{
    return &ctx->p_tokens[ctx->curr];
}

static const token *take_token(parse_ctx *ctx)
{
    return &ctx->p_tokens[ctx->curr++];
}

static mod_type *parse_type(parse_ctx *ctx)
{
    const token *t = nullptr;

    if ((t = take_token(ctx))->tok != TOKEN_ID) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "data type: type name expected, found %s",
                token_name(t->tok));
        return nullptr;
    }

    auto *e = new mod_type();
    set_node(e, t);
    e->name = std::string(t->text);

    return e;
}

static mod_expr *parse_expr(parse_ctx *ctx)
{
    const token *t = nullptr;

    if ((t = take_token(ctx))->tok != TOKEN_NUMBER) {
        compiler_error_at(
                ctx->parent_ctx, t->lnum, t->cnum, "invalid expression", token_name(t->tok));
        return nullptr;
    }

    auto *e = new mod_expr_value();
    set_node(e, t);
    e->text = std::string(t->text);

    return e;
}

static mod_function *parse_function(parse_ctx *ctx)
{
    const token *t = nullptr;

    if ((t = take_token(ctx))->tok != KW_FUNC) {
        compiler_error_at(ctx->parent_ctx, t->lnum, t->cnum, "function expected");
        return nullptr;
    }

    if ((t = take_token(ctx))->tok != TOKEN_ID) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function name expected, found %s",
                token_name(t->tok));
        return nullptr;
    }

    const token *t_id = t;

    if ((t = take_token(ctx))->tok != TOKEN_LPAREN) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function argument list: expected '(', found %s",
                token_name(t->tok));
        return nullptr;
    }

    // Arguments

    if ((t = take_token(ctx))->tok != TOKEN_RPAREN) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function argument list: expected ')', found %s",
                token_name(t->tok));
        return nullptr;
    }

    const mod_type *data_type = nullptr;
    if ((t = peek_token(ctx))->tok == TOKEN_COLON) {
        ctx->curr++;
        data_type = parse_type(ctx);
        if (!data_type) {
            return nullptr;
        }
    }

    if ((t = take_token(ctx))->tok != TOKEN_LCURLY) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function: expected '{', found %s",
                token_name(t->tok));
        return nullptr;
    }

    if ((t = take_token(ctx))->tok != TOKEN_RCURLY) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function: expected '}', found %s",
                token_name(t->tok));
        return nullptr;
    }

    auto *e = new mod_function();
    set_node(e, t_id);
    e->name = std::string(t_id->text);

    printf("function: %.*s\n", (int) t_id->text.size(), t_id->text.data());
    return e;
}

static mod_variable *parse_variable(parse_ctx *ctx)
{
    const token *t = nullptr;

    bool auto_var = false;
    if ((t = peek_token(ctx))->tok == KW_AUTO) {
        auto_var = true;
        ctx->curr++;
    }

    if ((t = take_token(ctx))->tok != KW_VAR) {
        compiler_error_at(ctx->parent_ctx, t->lnum, t->cnum, "variable expected");
        return nullptr;
    }

    if ((t = take_token(ctx))->tok != TOKEN_ID) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "variable name expected, found %s",
                token_name(t->tok));
        return nullptr;
    }

    const token *t_id = t;

    // Optional type
    mod_type *data_type = nullptr;
    if ((t = peek_token(ctx))->tok == TOKEN_COLON) {
        ctx->curr++;
        data_type = parse_type(ctx);
        if (!data_type) {
            return nullptr;
        }
    }

    // Optional initializer
    mod_expr *init = nullptr;
    if ((t = peek_token(ctx))->tok == TOKEN_EQ) {
        ctx->curr++;
        init = parse_expr(ctx);
        if (!init) {
            return nullptr;
        }
    }

    if ((t = take_token(ctx))->tok != TOKEN_SEMICOLON) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "variable: ';' expected, found %s",
                token_name(t->tok));
        return nullptr;
    }

    auto *e = new mod_variable();
    set_node(e, t_id);
    e->name = std::string(t_id->text);
    e->data_type = data_type;
    e->init = init;
    e->auto_var = auto_var;

    printf("variable: %.*s\n", (int) t_id->text.size(), t_id->text.data());
    return e;
}

static mod_object *parse_object_def(parse_ctx *ctx)
{
    const token *t = nullptr;

    if ((t = take_token(ctx))->tok != KW_OBJECT) {
        compiler_error_at(ctx->parent_ctx, t->lnum, t->cnum, "object def expected");
        return nullptr;
    }

    if ((t = take_token(ctx))->tok != TOKEN_ID) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "type name expected, found %s",
                token_name(t->tok));
        return nullptr;
    }

    const token *t_id = t;

    if ((t = take_token(ctx))->tok != TOKEN_LCURLY) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "object: expected '{', found %s",
                token_name(t->tok));
        return nullptr;
    }

    while (peek_token(ctx)->tok != TOKEN_RCURLY) {
        auto *field = parse_variable(ctx);
        if (!field) {
            return nullptr;
        }
        printf("field: %.*s\n", (int) field->name.size(), field->name.data());
    }

    if ((t = take_token(ctx))->tok != TOKEN_RCURLY) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "object: expected '}', found %s",
                token_name(t->tok));
        return nullptr;
    }

    auto *e = new mod_object();
    set_node(e, t_id);
    e->name = std::string(t_id->text);

    printf("object: %.*s\n", (int) t_id->text.size(), t_id->text.data());
    return e;
}

static bool parse_top_level_def(parse_ctx *ctx, mod_unit *unit)
{
    const token *t = peek_token(ctx);

    // Look ahead
    if (t->tok == KW_AUTO) {
        t = &ctx->p_tokens[ctx->curr + 1];
    }

    switch (t->tok) {
    case TOKEN_EOF:
        return true;

    case KW_FUNC: {
        auto *e = parse_function(ctx);
        if (e) {
            unit->v_function.push_back(e);
        }
        return e != nullptr;
    }

    case KW_VAR: {
        auto *e = parse_variable(ctx);
        if (e) {
            unit->v_variable.push_back(e);
        }
        return e != nullptr;
    }

    case KW_OBJECT: {
        auto *e = parse_object_def(ctx);
        if (e) {
            unit->v_object.push_back(e);
        }
        return e != nullptr;
    }

    default:
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "unrecognized top level construct starting with %s",
                token_name(t->tok));
        return false;
    }
}

static mod_unit *parse_unit(parse_ctx *ctx)
{
    auto *unit = new mod_unit();

    const token *t = peek_token(ctx);
    while (t->tok != TOKEN_EOF && parse_top_level_def(ctx, unit)) {
        t = peek_token(ctx);
    }

    if (t->tok != TOKEN_EOF) {
        delete unit;
        return nullptr;
    }

    return unit;
}

mod_unit *parse(context *ctx, const token *p_tokens, size_t n_tokens)
{
    parse_ctx parse_ctx = {};
    parse_ctx.parent_ctx = ctx;
    parse_ctx.p_tokens = p_tokens;
    parse_ctx.n_tokens = n_tokens;
    parse_ctx.curr = 0;

    return parse_unit(&parse_ctx);
}

void destroy_unit(context *ctx, mod_unit *unit)
{
    if (!unit) {
        return;
    }

    // TODO: Visit and delete
}

}
