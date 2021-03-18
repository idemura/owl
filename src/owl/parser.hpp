#ifndef OWL_PARSER_HPP
#define OWL_PARSER_HPP

#include "owl/lexer.hpp"
#include "owl/model.hpp"

/**
 * Parser. Build parse tree (model) from a stream of tokens.
 */

namespace owl {

mod_unit *parse(context *ctx, const token *p_tokens, size_t n_tokens);

}

#endif
