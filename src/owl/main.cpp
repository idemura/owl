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
        owl::compile_file(&ctx, argv[i]);
    }

    return ctx.n_errors > 0 ? 1 : 0;
}
