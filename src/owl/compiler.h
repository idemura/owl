#ifndef OWL_COMPILER_H
#define OWL_COMPILER_H

#include "foundation/string.h"

#include <stdio.h>

typedef struct {
    int n_errors;
    FILE *f_out;
} owl_context;

void owl_init_context(owl_context *ctx);
void owl_compile_file(owl_context *ctx, const char *file_name);

#endif
