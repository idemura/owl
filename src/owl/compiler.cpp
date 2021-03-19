#include "owl/compiler.hpp"

#include "owl/deduce_types.hpp"
#include "owl/parser.hpp"

namespace owl {

// Check if we have a simple ASCII charset.
static bool check_charset(context *ctx, std::string_view code)
{
    int lnum = 1;
    size_t line_first = 0;
    for (size_t i = 0; i < code.size(); i++) {
        bool valid = true;
        char c = code[i];
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
            compiler_error_at(ctx,
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

static std::string read_file(context *ctx, const char *file_name)
{
    auto *f = fopen(file_name, "rt");
    if (!f) {
        compiler_error(ctx, "failed to open file '%s'\n", file_name);
        return std::string();
    }

    fseek(f, 0L, SEEK_END);
    long size = ftell(f);
    fseek(f, 0L, SEEK_SET);

    auto buf = std::string(size, ' ');
    size_t read_size = fread(buf.data(), 1, size, f);
    if (read_size != size) {
        compiler_error(ctx, "failed to read file '%s': %zu size %zu\n", file_name, read_size, size);
        return std::string();
    }

    return buf;
}

bool compile_file(context *ctx, const char *file_name)
{
    std::string code = read_file(ctx, file_name);
    if (code.empty()) {
        return false;
    }

    ctx->file_name = std::string(file_name);
    if (!check_charset(ctx, code)) {
        return false;
    }

    return compile_string(ctx, code);
}

bool compile_string(context *ctx, std::string_view code)
{
    bool result = false;
    std::vector<token> tokens;

    mod_unit *unit = nullptr;
    if (tokenize(ctx, code, &tokens)) {
        unit = parse(ctx, tokens.data(), tokens.size());
        if (unit) {
            result = deduce_types(ctx, unit);
        }
    }

    destroy_rec(unit);

    return result;
}

} // owl
