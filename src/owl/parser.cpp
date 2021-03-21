#include "owl/parser.hpp"

#include <iostream>

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

static bool is_word(const token *t, const char *word)
{
    return t->tok == TOKEN_WORD && t->text == word;
}

static bool is_identifier(const token *t)
{
    return t->tok == TOKEN_WORD && !is_keyword(t->text);
}

static mod_type *parse_type(parse_ctx *ctx)
{
    const token *t = nullptr;

    if (!is_identifier(t = take_token(ctx))) {
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

static mod_stmt_return *parse_return(parse_ctx *ctx)
{
    const token *t = take_token(ctx);

    if (!is_word(t, KW_RETURN)) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "return statement expected, found %s",
                token_name(t->tok));
        return nullptr;
    }

    auto *e = new mod_stmt_return();
    set_node(e, t);

    e->expr = parse_expr(ctx);
    if (!e->expr) {
        destroy_rec(e);
        return nullptr;
    }

    return e;
}

static mod_body *parse_body(parse_ctx *ctx, const char *parent_entity)
{
    const token *t = nullptr;
    auto *e = new mod_body();

    if ((t = take_token(ctx))->tok != TOKEN_LCURLY) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "%s: expected '{', found %s",
                parent_entity,
                token_name(t->tok));
        destroy_rec(e);
        return nullptr;
    }

    while ((t = peek_token(ctx))->tok != TOKEN_RCURLY) {
        mod_node *stmt = nullptr;
        bool recognized = false;
        if (t->tok == TOKEN_WORD) {
            switch (t->text[0]) {
            case 'r':
                if (t->text == KW_RETURN) {
                    stmt = parse_return(ctx);
                    recognized = true;
                }
                break;
            }
        }

        if (!recognized) {
            // TODO: Parse expression
            assert(false);
        }

        if (!stmt) {
            compiler_error_at(ctx->parent_ctx,
                    t->lnum,
                    t->cnum,
                    "%s: statement expected, found %s",
                    parent_entity,
                    token_name(t->tok));
            destroy_rec(e);
            return nullptr;
        }

        e->statements.push_back(e);
    }

    if ((t = take_token(ctx))->tok != TOKEN_RCURLY) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "%s: expected '}', found %s",
                parent_entity,
                token_name(t->tok));
        destroy_rec(e);
        return nullptr;
    }

    return e;
}

static mod_function *parse_function(parse_ctx *ctx)
{
    const token *t = nullptr;

    if (!is_word(t = take_token(ctx), KW_FUNC)) {
        compiler_error_at(ctx->parent_ctx, t->lnum, t->cnum, "function expected");
        return nullptr;
    }

    if (!is_identifier(t = take_token(ctx))) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function name expected, found %s",
                token_name(t->tok));
        return nullptr;
    }

    auto *e = new mod_function();
    set_node(e, t);
    e->name = std::string(t->text);

    if ((t = take_token(ctx))->tok != TOKEN_LPAREN) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function argument list: expected '(', found %s",
                token_name(t->tok));
        destroy_rec(e);
        return nullptr;
    }

    // Arguments

    if ((t = take_token(ctx))->tok != TOKEN_RPAREN) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "function argument list: expected ')', found %s",
                token_name(t->tok));
        destroy_rec(e);
        return nullptr;
    }

    if ((t = peek_token(ctx))->tok == TOKEN_COLON) {
        ctx->curr++;
        e->data_type = parse_type(ctx);
        if (!e->data_type) {
            destroy_rec(e);
            return nullptr;
        }
    }

    e->body = parse_body(ctx, "function");
    if (!e->body) {
        destroy_rec(e);
        return nullptr;
    }

    std::cout << "function: " << e->name << "\n";
    return e;
}

static mod_variable *parse_variable(parse_ctx *ctx)
{
    const token *t = nullptr;

    bool auto_var = false;
    if (is_word(t = peek_token(ctx), KW_AUTO)) {
        auto_var = true;
        ctx->curr++;
    }

    if (!is_word(t = take_token(ctx), KW_VAR)) {
        compiler_error_at(ctx->parent_ctx, t->lnum, t->cnum, "variable expected");
        return nullptr;
    }

    if ((t = take_token(ctx))->tok != TOKEN_WORD) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "variable name expected, found %s",
                token_name(t->tok));
        return nullptr;
    }

    auto *e = new mod_variable();
    set_node(e, t);
    e->name = std::string(t->text);
    e->auto_var = auto_var;

    // Optional type
    if ((t = peek_token(ctx))->tok == TOKEN_COLON) {
        ctx->curr++;
        e->data_type = parse_type(ctx);
        if (!e->data_type) {
            destroy_rec(e);
            return nullptr;
        }
    }

    // Optional initializer
    if ((t = peek_token(ctx))->tok == TOKEN_EQ) {
        ctx->curr++;
        e->init_expr = parse_expr(ctx);
        if (!e->init_expr) {
            destroy_rec(e);
            return nullptr;
        }
    }

    if ((t = take_token(ctx))->tok != TOKEN_SEMICOLON) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "variable: ';' expected, found %s",
                token_name(t->tok));
        destroy_rec(e);
        return nullptr;
    }

    std::cout << "variable: " << e->name << "\n";
    return e;
}

static mod_object *parse_object_def(parse_ctx *ctx)
{
    const token *t = nullptr;

    if (!is_word(t = take_token(ctx), KW_OBJECT)) {
        compiler_error_at(ctx->parent_ctx, t->lnum, t->cnum, "object def expected");
        return nullptr;
    }

    if ((t = take_token(ctx))->tok != TOKEN_WORD) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "type name expected, found %s",
                token_name(t->tok));
        return nullptr;
    }

    auto *e = new mod_object();
    set_node(e, t);
    e->name = std::string(t->text);

    if ((t = take_token(ctx))->tok != TOKEN_LCURLY) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "object: expected '{', found %s",
                token_name(t->tok));
        destroy_rec(e);
        return nullptr;
    }

    while (peek_token(ctx)->tok != TOKEN_RCURLY) {
        auto *f = parse_variable(ctx);
        if (!f) {
            destroy_rec(e);
            return nullptr;
        }
        e->fields.push_back(f);
    }

    if ((t = take_token(ctx))->tok != TOKEN_RCURLY) {
        compiler_error_at(ctx->parent_ctx,
                t->lnum,
                t->cnum,
                "object: expected '}', found %s",
                token_name(t->tok));
        destroy_rec(e);
        return nullptr;
    }

    std::cout << "object: " << e->name << "\n";
    return e;
}

static bool parse_top_level_def(parse_ctx *ctx, mod_unit *unit)
{
    const token *t = peek_token(ctx);

    // Look ahead
    int adv = 0;
    if (is_word(t, KW_AUTO)) {
        t = &ctx->p_tokens[ctx->curr + 1];
        adv++;
    }

    if (t->tok == TOKEN_EOF) {
        return adv == 0;
    }

    // Step back where we stared after we found expected keyword position
    ctx->curr -= adv;

    switch (t->text[0]) {
    case KW_FUNC[0]:
        if (t->text == KW_FUNC) {
            auto *e = parse_function(ctx);
            if (e) {
                unit->functions.push_back(e);
            }
            return e != nullptr;
        }
        break;

    case KW_VAR[0]:
        if (t->text == KW_VAR) {
            auto *e = parse_variable(ctx);
            if (e) {
                unit->variables.push_back(e);
            }
            return e != nullptr;
        }
        break;

    case KW_OBJECT[0]:
        if (t->text == KW_OBJECT) {
            auto *e = parse_object_def(ctx);
            if (e) {
                unit->objects.push_back(e);
            }
            return e != nullptr;
        }
        break;
    }

    compiler_error_at(ctx->parent_ctx,
            t->lnum,
            t->cnum,
            "unrecognized top level construct starting with %s",
            token_name(t->tok));
    return false;
}

static mod_unit *parse_unit(parse_ctx *ctx)
{
    auto *e = new mod_unit();

    const token *t = peek_token(ctx);
    while (t->tok != TOKEN_EOF && parse_top_level_def(ctx, e)) {
        t = peek_token(ctx);
    }

    if (t->tok != TOKEN_EOF) {
        destroy_rec(e);
        return nullptr;
    }

    return e;
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

} // owl
