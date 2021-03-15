#ifndef OWL_MODEL_H
#define OWL_MODEL_H

#include "foundation/string.h"
#include "foundation/vector.h"

/**
 * Model. Parse tree nodes.
 */

typedef enum {
    OWL_MN_UNIT,
    OWL_MN_SIZE,
} owl_node_t;

typedef struct {
    string name;
} owl_func;

typedef def_vector(owl_func *) vector_owl_func;

typedef struct {
    vector_owl_func funcs;
} owl_unit;

#endif
