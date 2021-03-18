#ifndef OWL_VISITOR_HPP
#define OWL_VISITOR_HPP

#include "owl/context.hpp"
#include "owl/model.hpp"

#include <string>
#include <string_view>
#include <vector>

/**
 * Model tree visitor.
 */

namespace owl {

struct visitor;

typedef void (*visitor_fn)(visitor *v, void *bind, mod_node *node);

struct visitor {
    context *ctx = nullptr;
    visitor_fn visit[MOD_SIZE] = {};

    explicit visitor(context *c);
};

void visit_children(visitor *v, void *bind, mod_node *node);
void visit(visitor *v, void *bind, mod_node *node);

}

#endif
