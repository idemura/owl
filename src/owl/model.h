#ifndef OWL_MODEL_H
#define OWL_MODEL_H

#include "foundation/string.h"
#include "foundation/vector.h"

#define OWL_MAX_ARGS 80
#define OWL_MAX_FIELDS 4096

/**
 * Model. Parse tree nodes.
 */

typedef enum {
    OWL_MN_FUNCTION,
    OWL_MN_VARIABLE,
    OWL_MN_OBJECT,
    OWL_MN_STRUCT,
    OWL_MN_EXPR,
    OWL_MN_UNIT,
    OWL_MN_SIZE,
} owl_node_t;

/**
 * Node base (contained in every node)
 */
typedef struct {
    owl_node_t type;
    int lnum;
    int cnum;
    string text;
} owl_node;

/**
 * Type definition.
 */
typedef struct {
    owl_node base;

    string name;

    bool is_builtin;
    bool is_ref;
} owl_type_def;

/**
 * Expression type, links to the type definition.
 */
typedef struct {
    owl_node base;

    string name;
    owl_type_def *type_def;
} owl_data_type;

/**
 * Base of every owl_expr_* data structure
 */
typedef struct {
    owl_node base;

    owl_data_type data_type;
} owl_expr;

/**
 * Function definition
 */
typedef struct {
    owl_node base;

    string name;
} owl_function;

/**
 * Variable/field definition
 */
typedef struct {
    owl_node base;

    string name;

    owl_data_type *data_type;
    owl_expr *init;

    bool auto_var;
} owl_variable;

/**
 * Object type definition
 */
typedef struct {
    owl_node base;

    string name;
} owl_object;

/**
 * Struct type definition
 */
typedef struct {
    owl_node base;

    string name;
} owl_struct;

/**
 * Function application
 */
typedef struct {
    owl_expr expr;

    string name;

    owl_expr *p_args;
    size_t n_args;
} owl_expr_func;

/**
 * Value (literal) in expression
 */
typedef struct {
    owl_expr expr;

    string text;
} owl_expr_value;

typedef def_vector(owl_function *) vector_owl_function;
typedef def_vector(owl_variable *) vector_owl_variable;
typedef def_vector(owl_object *) vector_owl_object;
typedef def_vector(owl_struct *) vector_owl_struct;

typedef struct {
    owl_node base;

    vector_owl_function v_function;
    vector_owl_variable v_variable;
    vector_owl_object v_object;
    vector_owl_struct v_struct;
} owl_unit;

#endif
