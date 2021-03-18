#include "owl/compiler.h"

#include "owl/parser.h"

// Check if we have a simple ASCII charset.
static bool check_charset(owl_context *ctx, string code)
{
    int lnum = 1;
    size_t line_first = 0;
    for (size_t i = 0; i < code.len; i++) {
        bool valid = true;
        char c = code.str[i];
        switch (c) {
        case '\n':
            lnum++;
            line_first = i + 1;
            break;

        case '\f':
        case '\r':
        case '\t':
        case '\v':
            break;

        default:
            valid = c >= ' ' && c <= 127;
            break;
        }
        if (!valid) {
            owl_error_at(ctx,
                    lnum,
                    i - line_first + 1,
                    "(charset check) invalid character: '%c' ord=%d",
                    c,
                    (int) c);
            return false;
        }
    }
    return true;
}

static string read_file(owl_context *ctx, const char *file_name)
{
    FILE *f = fopen(file_name, "rt");
    if (!f) {
        owl_error(ctx, "failed to open file '%s'\n", file_name);
        return string_empty();
    }

    fseek(f, 0L, SEEK_END);
    long size = ftell(f);
    fseek(f, 0L, SEEK_SET);

    char *buf = memmgr_allocate_dirty(ctx->mmc, size + 1);
    size_t read_size = fread(buf, 1, size, f);
    if (read_size != size) {
        owl_error(ctx, "failed to read file '%s': %zu size %zu\n", file_name, read_size, size);
        goto fail;
    }

    buf[size] = 0;
    return string_of_len(buf, size);

fail:
    memmgr_release(ctx->mmc, buf);
    return string_empty();
}

bool owl_compile_file(owl_context *ctx, const char *file_name)
{
    bool result = false;

    string code = read_file(ctx, file_name);
    if (code.len == 0) {
        goto leave;
    }

    ctx->file_name = file_name;
    if (!check_charset(ctx, code)) {
        goto leave;
    }

    result = owl_compile_string(ctx, code);

leave:
    memmgr_release(ctx->mmc, code.str);
    ctx->file_name = NULL;

    return result;
}

bool owl_compile_string(owl_context *ctx, string code)
{
    bool result = false;

    vector_owl_token tokens;
    vector_init_with_ctx(&tokens, ctx->mmc);

    owl_unit *unit = NULL;
    if (owl_tokenize(ctx, code, &tokens)) {
        unit = owl_parse(ctx, vector_ptr(&tokens, 0), vector_size(&tokens));
        if (unit) {
            result = true;
        }
    }

    owl_destroy_unit(ctx, unit);
    vector_release(&tokens);

    return result;
}
