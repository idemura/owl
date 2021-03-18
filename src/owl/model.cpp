#include "owl/model.hpp"

namespace owl {

void mod_function::destroy_rec()
{
    if (data_type) {
        data_type->destroy_rec();
    }
    delete this;
}

void mod_variable::destroy_rec()
{
    if (data_type) {
        data_type->destroy_rec();
    }
    if (init_expr) {
        init_expr->destroy_rec();
    }
    delete this;
}

void mod_object::destroy_rec()
{
    for (auto *e : fields) {
        e->destroy_rec();
    }
    delete this;
}

void mod_struct::destroy_rec()
{
    delete this;
}

void mod_expr::destroy_rec()
{
    if (data_type) {
        data_type->destroy_rec();
    }
    delete this;
}

void mod_expr_func::destroy_rec()
{
    for (auto *e : args) {
        e->destroy_rec();
    }
    delete this;
}

void mod_unit::destroy_rec()
{
    for (auto *e : functions) {
        e->destroy_rec();
    }
    for (auto *e : variables) {
        e->destroy_rec();
    }
    for (auto *e : objects) {
        e->destroy_rec();
    }
    for (auto *e : structs) {
        e->destroy_rec();
    }
    delete this;
}

void destroy_rec(mod_node *node)
{
    if (node) {
        node->destroy_rec();
    }
}

}
