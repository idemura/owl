#ifndef OWL_COMPILER_HPP
#define OWL_COMPILER_HPP

#include <string>
#include <vector>

#include "owl/context.hpp"
#include "owl/lexer.hpp"

namespace owl {

bool compile_file(owl_context *ctx, const char *file_name);
bool compile_string(owl_context *ctx, string code);

}

#endif
