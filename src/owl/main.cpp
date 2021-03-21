#include "owl/compiler.hpp"

#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc == 1) {
        printf("Owl programming language compiler\n"
               "Usage:\n"
               "  owl file...\n");
        return 0;
    }

    owl::context ctx;
    // ctx.debug_lexer = true;

    for (int i = 1; i < argc; i++) {
        const char *file_name = argv[i];
        if (!owl::compile_file(&ctx, file_name)) {
            fprintf(stderr, "Failed to compile '%s'\n", file_name);
        }
    }

    if (ctx.n_errors == 0) {
        fprintf(stdout, "Compilation successful\n");
    }
    return ctx.n_errors > 0 ? 1 : 0;
}
