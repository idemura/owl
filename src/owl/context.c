#include "owl/context.h"

void owl_context_init(owl_context *ctx)
{
    ctx->f_error = stderr;
    ctx->f_debug = stdout;
    ctx->n_errors = 0;
    ctx->file_name = NULL;
    ctx->mmc = NULL;
    ctx->debug_lexer = false;
}

void owl_context_destroy(owl_context *ctx)
{
    if (ctx->mmc && ctx->mmc->n_allocs > 0) {
        die("%ld allocations are not released", ctx->mmc->n_allocs);
    }
}

void owl_error_va(owl_context *ctx, int lnum, int cnum, const char *format, va_list va)
{
    if (ctx->file_name) {
        fprintf(ctx->f_error, "In %s", ctx->file_name);
        if (lnum > 0) {
            fprintf(ctx->f_error, ":%d", lnum);
            if (cnum > 0) {
                fprintf(ctx->f_error, ":%d", cnum);
            }
        }
        fprintf(ctx->f_error, ": ");
    }

    fprintf(ctx->f_error, "error: ");
    vfprintf(ctx->f_error, format, va);
    fprintf(ctx->f_error, "\n");

    ctx->n_errors++;
}

void owl_error_at(owl_context *ctx, int lnum, int cnum, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    owl_error_va(ctx, lnum, cnum, format, va);
    va_end(va);
}

void owl_error(owl_context *ctx, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    owl_error_va(ctx, 0, 0, format, va);
    va_end(va);
}
