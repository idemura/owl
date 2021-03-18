#ifndef OWL_CONTEXT_HPP
#define OWL_CONTEXT_HPP

#include <string>

#include <stdio.h>

namespace owl {

struct owl_context {
    FILE *f_error = nullptr;
    FILE *f_debug = nullptr;

    int n_errors = 0;
    const char *file_name = nullptr;

    memmgr_ctx *mmc;

    // Parameters
    bool debug_lexer;
};

void owl_error_va(owl_context *ctx, int lnum, int cnum, const char *format, va_list va);
void owl_error(owl_context *ctx, const char *format, ...);
void owl_error_at(owl_context *ctx, int lnum, int cnum, const char *format, ...);

}

#endif
