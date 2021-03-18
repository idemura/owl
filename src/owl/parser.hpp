#ifndef OWL_PARSER_HPP
#define OWL_PARSER_HPP

#include "owl/lexer.hpp"
#include "owl/model.hpp"

/**
 * Parser. Build parse tree (model) from a stream of tokens.
 */

namespace owl {

owl_unit *owl_parse(owl_context *ctx, const owl_token *p_tokens, size_t n_tokens);
void owl_destroy_unit(owl_context *ctx, owl_unit *unit);

}

#endif
