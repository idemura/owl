#ifndef OWL_MODEL_HPP
#define OWL_MODEL_HPP

#include <string>
#include <string_view>
#include <vector>

#define OWL_MAX_ARGS 80
#define OWL_MAX_FIELDS 4096

/**
 * Model. Parse tree nodes.
 */

namespace owl {

enum mod_node_t {
    MOD_NULL,
    MOD_FUNCTION,
    MOD_VARIABLE,
    MOD_OBJECT,
    MOD_STRUCT,
    MOD_TYPE_DEF,
    MOD_TYPE,
    MOD_EXPR,
    MOD_UNIT,
    MOD_SIZE,
};

/**
 * Node base (contained in every node)
 */
struct mod_node {
    const mod_node_t type = MOD_NULL;

    int lnum = 0;
    int cnum = 0;
    std::string_view text;

    mod_node(mod_node_t t): type{t} {}
};

/**
 * Type definition.
 */
struct mod_type_def: mod_node {
    std::string name;

    bool is_builtin = false;
    bool is_ref = false;

    mod_type_def(): mod_node(MOD_TYPE_DEF) {}
};

/**
 * Expression type, links to the type definition.
 */
struct mod_type: mod_node {
    std::string name;
    mod_type_def *type_def = nullptr;

    mod_type(): mod_node(MOD_TYPE) {}
};

/**
 * Base of every mod_expr_* data structure
 */
struct mod_expr: mod_node {
    mod_type data_type;

    mod_expr(): mod_node(MOD_EXPR) {}
};

/**
 * Function definition
 */
struct mod_function: mod_node {
    std::string name;

    mod_function(): mod_node(MOD_FUNCTION) {}
};

/**
 * Variable/field definition
 */
struct mod_variable: mod_node {
    std::string name;

    mod_type *data_type = nullptr;
    mod_expr *init = nullptr;

    bool auto_var = false;

    mod_variable(): mod_node(MOD_VARIABLE) {}
};

/**
 * Object type definition
 */
struct mod_object: mod_node {
    std::string name;

    mod_object(): mod_node(MOD_OBJECT) {}
};

/**
 * Struct type definition
 */
struct mod_struct: mod_node {
    std::string name;

    mod_struct(): mod_node(MOD_STRUCT) {}
};

/**
 * Function application
 */
struct mod_expr_func: mod_expr {
    std::string name;
    std::vector<mod_variable *> args;
};

/**
 * Value (literal) in expression
 */
struct mod_expr_value: mod_expr {
    std::string text;
};

struct mod_unit: mod_node {
    std::vector<mod_function *> v_function;
    std::vector<mod_variable *> v_variable;
    std::vector<mod_object *> v_object;
    std::vector<mod_struct *> v_struct;

    mod_unit(): mod_node(MOD_UNIT) {}
};

}

#endif
