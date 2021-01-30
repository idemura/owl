#ifndef TESTING_TESTING_H
#define TESTING_TESTING_H

#include "foundation/lang.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK_VA(cond, ...) \
    do { \
        if (!(cond)) { \
            testing_fail(__func__, __FILE__, __LINE__, #cond, __VA_ARGS__); \
            return; \
        } \
    } while (0);

#define CHECK(cond) CHECK_VA(cond, NULL)

#define TEST_CASE(func) \
    (test_case) { .body = &(func), #func }

typedef struct {
    void (*body)(void);
    const char *name;
} test_case;

void testing_init(int argc, char **argv);
void testing_add(void (*begin)(void), void (*end)(void), test_case *tests, size_t n_tests);
bool testing_run(void);
void testing_finish(void);

/* Internal */
void testing_fail(
        const char *func, const char *file, int line, const char *expr, const char *message, ...);

#endif
