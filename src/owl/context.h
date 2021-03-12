#ifndef OWL_CONTEXT_H
#define OWL_CONTEXT_H

#include "foundation/string.h"

#include <stdio.h>

#ifdef NDEBUG
#define MMC(ctx) NULL
#else
#define MMC(ctx) (&(ctx)->mmc)
#endif

typedef struct {
    FILE *f_error;
    FILE *f_debug;

    int n_errors;
    const char *file_name;

    memmgr_ctx mmc;

    // Parameters
    bool debug_lexer;

} owl_context;

void owl_context_init(owl_context *ctx);
void owl_context_destroy(owl_context *ctx);

void owl_error_va(owl_context *ctx, int lnum, int cnum, const char *format, va_list va);
void owl_error(owl_context *ctx, const char *format, ...);
void owl_error_at(owl_context *ctx, int lnum, int cnum, const char *format, ...);

#endif
