#include "owl/compiler.h"

void owl_init_context(owl_context *ctx)
{
    ctx->n_errors = 0;
    ctx->f_out = stdout;
}

static string read_file(owl_context *ctx, const char *file_name)
{
    FILE *f = fopen(file_name, "rt");
    if (!f) {
        fprintf(ctx->f_out, "Failed to open file '%s'\n", file_name);
        return string_empty();
    }

    fseek(f, 0L, SEEK_END);
    long size = ftell(f);
    fseek(f, 0L, SEEK_SET);

    printf("%ld\n", size);
    return string_empty();
}

void owl_compile_file(owl_context *ctx, const char *file_name)
{
    string src = read_file(ctx, file_name);
    if (src.len == 0) {
        return;
    }

    printf("compiling %s\n", file_name);

    string_release(src);
}
