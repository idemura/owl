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

typedef mod_node *(*visit_fn)(const visitor *v, void *bind, mod_node *node);

struct visitor {
    context *root_ctx = nullptr;
    visit_fn visit[MOD_SIZE] = {};

    explicit visitor(context *ctx);
};

void visit_children(const visitor *v, void *bind, mod_node *node);
mod_node *visit(const visitor *v, void *bind, mod_node *node);

} // owl

#endif
