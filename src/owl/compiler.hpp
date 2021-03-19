#ifndef OWL_COMPILER_HPP
#define OWL_COMPILER_HPP

#include "owl/context.hpp"
#include "owl/lexer.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace owl {

bool compile_file(context *ctx, const char *file_name);
bool compile_string(context *ctx, std::string_view code);

} // owl

#endif
