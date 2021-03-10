#include "foundation/call_stack.h"
#include "owl/compiler.h"

#include <stdio.h>

int main(int argc, char **argv)
{
    call_stack_init(argv[0]);

    if (argc == 1) {
        printf("Owl programming language compiler\n"
               "Usage:\n"
               "  owl file...\n");
        return 0;
    }

    owl_context ctx;
    owl_init_context(&ctx);

    for (int i = 1; i < argc; i++) {
        owl_compile_file(&ctx, argv[i]);
    }

    return ctx.n_errors > 0 ? 1 : 0;
}
