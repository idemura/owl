#ifndef OWL_COMPILER_H
#define OWL_COMPILER_H

#include "foundation/string.h"
#include "foundation/vector.h"
#include "owl/context.h"
#include "owl/lexer.h"

bool owl_compile_file(owl_context *ctx, const char *file_name);
bool owl_compile_string(owl_context *ctx, string code);

typedef struct {
    // Parent context
    owl_context *ctx;

    vector_owl_token tokens;
} owl_parse_ctx;

#endif
