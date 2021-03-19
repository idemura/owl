#ifndef OWL_DEDUCE_TYPES_HPP
#define OWL_DEDUCE_TYPES_HPP

#include "owl/context.hpp"

/**
 * Deduce entity types.
 */

namespace owl {

struct mod_node;

bool deduce_types(context *ctx, mod_node *node);

}

#endif
