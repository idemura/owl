#include "testing/testing.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_MAX 1200

static const char test_prefix[] = "test_";

typedef struct {
    // Called before the first test in suite
    void (*begin)(void);

    // Called after the last test
    void (*end)(void);

    test_case *tests;
    size_t n_tests;
} test_suite;

static int n_failed_checks;
static int test_suite_reg_size;
static test_suite test_suite_reg[TEST_MAX];

void testing_init(int argc, char **argv)
{
    // Empty
}

void testing_add(void (*begin)(void), void (*end)(void), test_case *tests, size_t n_tests)
{
    if (test_suite_reg_size == TEST_MAX) {
        fprintf(stderr, "Test case test_suite_registry memory exceeded\n");
        exit(1);
    }
    test_suite_reg[test_suite_reg_size++] = (test_suite){begin, end, tests, n_tests};
}

bool testing_run(void)
{
    const size_t prefix_len = strlen(test_prefix);

    int n_failed_tests = 0;
    int n_tests = 0;
    for (int i = 0; i < test_suite_reg_size; i++) {
        test_suite *entry = &test_suite_reg[i];
        if (entry->begin) {
            entry->begin();
        }

        for (int j = 0; j < entry->n_tests; j++) {
            const char *name = entry->tests[j].name;
            if (strncmp(name, test_prefix, prefix_len) == 0) {
                name += prefix_len;
            }

            int f = n_failed_checks;
            entry->tests[j].body();
            if (n_failed_checks > f) {
                fprintf(stderr, "FAILED %s\n", name);
                n_failed_tests++;
            } else {
                fprintf(stderr, "Passed %s\n", name);
            }
            n_tests++;
        }
        if (entry->end) {
            entry->end();
        }
    }
    if (n_failed_tests) {
        fprintf(stderr, "Failed: %u/%u tests\n", n_failed_tests, n_tests);
    } else {
        fprintf(stderr, "All tests PASSED\n");
    }
    return n_failed_tests > 0;
}

void testing_finish(void)
{
    // Empty. Release memory.
}

void testing_fail(
        const char *func, const char *file, int line, const char *expr, const char *message, ...)
{
    fprintf(stderr, "Check failed in %s %s@%d: %s\n", func, file, line, expr);

    va_list va;
    if (message) {
        fprintf(stderr, "  ");

        va_start(va, message);
        vfprintf(stderr, message, va);
        va_end(va);

        fprintf(stderr, "\n");
    }

    n_failed_checks++;
}
