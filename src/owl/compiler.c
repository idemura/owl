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

    char *buf = memmgr_allocate_dirty(NULL, size + 1);
    size_t read_size = fread(buf, 1, size, f);
    if (read_size != size) {
        fprintf(ctx->f_out, "Failed to read file '%s': %zu size %zu\n", file_name, read_size, size);
        goto fail;
    }

    buf[size] = 0;
    return string_of_len(buf, size);

fail:
    memmgr_release(NULL, buf);
    return string_empty();
}

void owl_compile_file(owl_context *ctx, const char *file_name)
{
    string src = read_file(ctx, file_name);
    if (src.len == 0) {
        return;
    }

    printf("compiling %s %zu\n", file_name, src.len);

    string_release(src);
}
