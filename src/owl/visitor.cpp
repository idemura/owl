#include "owl/visitor.hpp"

namespace owl {

static void no_children(const visitor *v, void *bind, mod_node *node)
{
}

static void children_of_function(const visitor *v, void *bind, mod_function *node)
{
    if (node->data_type) {
        auto *r = visit(v, bind, node->data_type);
        if (r) {
            node->data_type = (mod_type *) r;
        }
    }
}

static void children_of_variable(const visitor *v, void *bind, mod_variable *node)
{
    if (node->data_type) {
        auto *r = visit(v, bind, node->data_type);
        if (r) {
            node->data_type = (mod_type *) r;
        }
    }
    if (node->init_expr) {
        auto *r = visit(v, bind, node->init_expr);
        if (r) {
            node->init_expr = (mod_expr *) r;
        }
    }
}

static void children_of_object(const visitor *v, void *bind, mod_object *node)
{
    for (size_t i = 0; i < node->fields.size(); i++) {
        auto *r = visit(v, bind, node->fields[i]);
        if (r) {
            node->fields[i] = (mod_variable *) r;
        }
    }
}

static void children_of_struct(const visitor *v, void *bind, mod_struct *node)
{
    // TODO
}

static void children_of_type(const visitor *v, void *bind, mod_type *node)
{
}

static void children_of_body(const visitor *v, void *bind, mod_body *node)
{
}

static void children_of_expr(const visitor *v, void *bind, mod_expr *node)
{
    if (node->data_type) {
        auto *r = visit(v, bind, node->data_type);
        if (r) {
            node->data_type = (mod_type *) r;
        }
    }
}

static void children_of_unit(const visitor *v, void *bind, mod_unit *node)
{
    for (size_t i = 0; i < node->functions.size(); i++) {
        auto *r = visit(v, bind, node->functions[i]);
        if (r) {
            node->functions[i] = (mod_function *) r;
        }
    }
    for (size_t i = 0; i < node->variables.size(); i++) {
        auto *r = visit(v, bind, node->variables[i]);
        if (r) {
            node->variables[i] = (mod_variable *) r;
        }
    }
    for (size_t i = 0; i < node->objects.size(); i++) {
        auto *r = visit(v, bind, node->objects[i]);
        if (r) {
            node->objects[i] = (mod_object *) r;
        }
    }
    for (size_t i = 0; i < node->structs.size(); i++) {
        auto *r = visit(v, bind, node->structs[i]);
        if (r) {
            node->structs[i] = (mod_struct *) r;
        }
    }
}

typedef void (*visit_children_fn)(const visitor *v, void *bind, mod_node *node);

static visit_children_fn children_of[MOD_SIZE] = {
        &no_children,
        (visit_children_fn) &children_of_function,
        (visit_children_fn) &children_of_variable,
        (visit_children_fn) &children_of_object,
        (visit_children_fn) &children_of_struct,
        (visit_children_fn) &children_of_type,
        (visit_children_fn) &children_of_body,
        (visit_children_fn) &children_of_expr,
        (visit_children_fn) &children_of_unit,
};

static mod_node *default_visit(const visitor *v, void *bind, mod_node *node)
{
    visit_children(v, bind, node);
    return nullptr;
}

visitor::visitor(context *ctx): root_ctx{ctx}
{
    for (int i = 0; i < MOD_SIZE; i++) {
        visit[i] = &default_visit;
    }
}

void visit_children(const visitor *v, void *bind, mod_node *node)
{
    children_of[node->type](v, bind, node);
}

mod_node *visit(const visitor *v, void *bind, mod_node *node)
{
    if (node) {
        return v->visit[node->type](v, bind, node);
    } else {
        return nullptr;
    }
}

} // owl
