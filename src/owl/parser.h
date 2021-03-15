#ifndef OWL_PARSER_H
#define OWL_PARSER_H

#include "owl/lexer.h"
#include "owl/model.h"

/**
 * Parser. Build parse tree (model) from a stream of tokens.
 */

bool owl_parse(owl_context *ctx, const owl_token *p_tokens, size_t n_tokens);

#endif
