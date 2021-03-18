#ifndef OWL_MODEL_HPP
#define OWL_MODEL_HPP

#include <string>
#include <string_view>
#include <vector>

/**
 * Model nodes. Model is a graph representing parsed program.
 */

namespace owl {

enum mod_node_t {
    MOD_NULL,
    MOD_FUNCTION,
    MOD_VARIABLE,
    MOD_OBJECT,
    MOD_STRUCT,
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

    explicit mod_node(mod_node_t t): type{t} {}
    virtual ~mod_node() = default;

    virtual void destroy_rec() { delete this; }
};

/**
 * Type definition.
 */
struct type_definition {
    std::string name;

    bool is_builtin = false;
    bool is_ref = false;
};

/**
 * Expression type, links to the type definition.
 */
struct mod_type: mod_node {
    std::string name;
    type_definition *type_def = nullptr;

    mod_type(): mod_node(MOD_TYPE) {}
};

/**
 * Base of every mod_expr_* data structure
 */
struct mod_expr: mod_node {
    mod_type *data_type = nullptr;

    mod_expr(): mod_node(MOD_EXPR) {}
    void destroy_rec() override;
};

/**
 * Function definition
 */
struct mod_function: mod_node {
    std::string name;
    mod_type *data_type = nullptr;

    mod_function(): mod_node(MOD_FUNCTION) {}
    void destroy_rec() override;
};

/**
 * Variable/field definition
 */
struct mod_variable: mod_node {
    std::string name;

    mod_type *data_type = nullptr;
    mod_expr *init_expr = nullptr;

    bool auto_var = false;

    mod_variable(): mod_node(MOD_VARIABLE) {}
    void destroy_rec() override;
};

/**
 * Object type definition
 */
struct mod_object: mod_node {
    std::string name;
    std::vector<mod_variable *> fields;

    mod_object(): mod_node(MOD_OBJECT) {}
    void destroy_rec() override;
};

/**
 * Struct type definition
 */
struct mod_struct: mod_node {
    std::string name;

    mod_struct(): mod_node(MOD_STRUCT) {}
    void destroy_rec() override;
};

/**
 * Function application
 */
struct mod_expr_func: mod_expr {
    std::string name;
    std::vector<mod_variable *> args;

    void destroy_rec() override;
};

/**
 * Value (literal) in expression
 */
struct mod_expr_value: mod_expr {
    std::string text;
};

struct mod_unit: mod_node {
    std::vector<mod_function *> functions;
    std::vector<mod_variable *> variables;
    std::vector<mod_object *> objects;
    std::vector<mod_struct *> structs;

    mod_unit(): mod_node(MOD_UNIT) {}
    void destroy_rec() override;
};

void destroy_rec(mod_node *node);

}

#endif
