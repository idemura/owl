#ifndef OWL_CONTEXT_HPP
#define OWL_CONTEXT_HPP

#include <stdio.h>

#include <string>
#include <string_view>

namespace owl {

struct context {
    FILE *f_error = stderr;
    FILE *f_debug = stdout;

    int n_errors = 0;
    std::string file_name;

    // Parameters
    bool debug_lexer;
};

void compiler_error_va(context *ctx, int lnum, int cnum, const char *format, va_list va);
void compiler_error(context *ctx, const char *format, ...);
void compiler_error_at(context *ctx, int lnum, int cnum, const char *format, ...);

} // owl

#endif
