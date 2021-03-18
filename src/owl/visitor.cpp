#include "owl/visitor.hpp"

namespace owl {

static void no_children(visitor *v, void *bind, mod_node *node)
{
}

static void children_of_function(visitor *v, void *bind, mod_function *node)
{
}

static void children_of_variable(visitor *v, void *bind, mod_variable *node)
{
    if (node->data_type) {
        visit(v, bind, node->data_type);
    }
    if (node->init_expr) {
        visit(v, bind, node->init_expr);
    }
}

static void children_of_object(visitor *v, void *bind, mod_object *node)
{
}

static void children_of_struct(visitor *v, void *bind, mod_struct *node)
{
}

static void children_of_type(visitor *v, void *bind, mod_type *node)
{
}

static void children_of_expr(visitor *v, void *bind, mod_expr *node)
{
}

static void children_of_unit(visitor *v, void *bind, mod_unit *node)
{
    for (auto *p : node->functions) {
        visit(v, bind, p);
    }
    for (auto *p : node->variables) {
        visit(v, bind, p);
    }
    for (auto *p : node->objects) {
        visit(v, bind, p);
    }
    for (auto *p : node->structs) {
        visit(v, bind, p);
    }
}

static visitor_fn children_of[MOD_SIZE] = {
        &no_children,
        (visitor_fn) &children_of_function,
        (visitor_fn) &children_of_variable,
        (visitor_fn) &children_of_object,
        (visitor_fn) &children_of_struct,
        (visitor_fn) &children_of_type,
        (visitor_fn) &children_of_expr,
        (visitor_fn) &children_of_unit,
};

visitor::visitor(context *ctx_): ctx{ctx_}
{
    for (int i = 0; i < MOD_SIZE; i++) {
        visit[i] = &visit_children;
    }
}

void visit_children(visitor *v, void *bind, mod_node *node)
{
    children_of[node->type](v, bind, node);
}

void visit(visitor *v, void *bind, mod_node *node)
{
    if (node) {
        v->visit[node->type](v, bind, node);
    }
}

}
