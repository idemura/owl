#include "owl/context.hpp"

namespace owl {

void compiler_error_va(context *ctx, int lnum, int cnum, const char *format, va_list va)
{
    if (!ctx->file_name.empty()) {
        fprintf(ctx->f_error, "In %s", ctx->file_name.data());
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

void compiler_error_at(context *ctx, int lnum, int cnum, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    compiler_error_va(ctx, lnum, cnum, format, va);
    va_end(va);
}

void compiler_error(context *ctx, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    compiler_error_va(ctx, 0, 0, format, va);
    va_end(va);
}

}
