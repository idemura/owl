#include "owl/model.hpp"

namespace owl {

void mod_expr::destroy_rec()
{
    if (data_type) {
        data_type->destroy_rec();
        data_type = nullptr;
    }
    mod_node::destroy_rec();
}

void mod_stmt::destroy_rec()
{
    mod_node::destroy_rec();
}

void mod_function::destroy_rec()
{
    if (data_type) {
        data_type->destroy_rec();
        data_type = nullptr;
    }

    if (body) {
        body->destroy_rec();
        body = nullptr;
    }

    mod_node::destroy_rec();
}

void mod_variable::destroy_rec()
{
    if (data_type) {
        data_type->destroy_rec();
        data_type = nullptr;
    }
    if (init_expr) {
        init_expr->destroy_rec();
        init_expr = nullptr;
    }
    mod_node::destroy_rec();
}

void mod_object::destroy_rec()
{
    for (auto *e : fields) {
        e->destroy_rec();
    }
    fields.clear();

    mod_node::destroy_rec();
}

void mod_struct::destroy_rec()
{
    mod_node::destroy_rec();
}

void mod_type::destroy_rec()
{
    mod_node::destroy_rec();
}

void mod_body::destroy_rec()
{
    for (auto *e : statements) {
        e->destroy_rec();
    }
    statements.clear();

    mod_node::destroy_rec();
}

void mod_stmt_return::destroy_rec()
{
    if (expr) {
        expr->destroy_rec();
        expr = nullptr;
    }

    mod_stmt::destroy_rec();
}

void mod_expr_apply::destroy_rec()
{
    for (auto *e : args) {
        e->destroy_rec();
    }
    args.clear();

    mod_expr::destroy_rec();
}

void mod_expr_value::destroy_rec()
{
    mod_expr::destroy_rec();
}

void mod_unit::destroy_rec()
{
    for (auto *e : functions) {
        e->destroy_rec();
    }
    functions.clear();

    for (auto *e : variables) {
        e->destroy_rec();
    }
    variables.clear();

    for (auto *e : objects) {
        e->destroy_rec();
    }
    objects.clear();

    for (auto *e : structs) {
        e->destroy_rec();
    }
    structs.clear();

    mod_node::destroy_rec();
}

void destroy_rec(mod_node *node)
{
    if (node) {
        node->destroy_rec();
    }
}

} // owl
