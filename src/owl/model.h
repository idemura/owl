#ifndef OWL_MODEL_H
#define OWL_MODEL_H

#include "foundation/string.h"
#include "foundation/vector.h"

/**
 * Model. Parse tree nodes.
 */

typedef enum {
    OWL_MN_FUNCTION,
    OWL_MN_VARIABLE,
    OWL_MN_CLASS,
    OWL_MN_VALUE,
    OWL_MN_UNIT,
    OWL_MN_SIZE,
} owl_node_t;

typedef struct {
    owl_node_t type;

    string name;
} owl_function;

typedef struct {
    owl_node_t type;

    string name;
} owl_variable;

typedef struct {
    owl_node_t type;

    string name;
} owl_class;

typedef struct {
    owl_node_t type;

    string name;
} owl_value;

typedef def_vector(owl_function *) vector_owl_function;
typedef def_vector(owl_variable *) vector_owl_variable;
typedef def_vector(owl_class *) vector_owl_class;
typedef def_vector(owl_value *) vector_owl_value;

typedef struct {
    owl_node_t type;

    vector_owl_function v_function;
    vector_owl_variable v_variable;
    vector_owl_class v_class;
    vector_owl_value v_value;
} owl_unit;

#endif
