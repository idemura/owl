#ifndef TESTING_TESTING_H
#define TESTING_TESTING_H

#include <stdbool.h>
#include <stddef.h>

#include "types/primitive.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*TestCaseFP)();

void testing_init(int argc, char **argv);
void testing_register(TestCaseFP func, char const *funcName);
bool testing_run();
void testing_finish();

// Internal:
void testing_fail(
    char const *funcName,
    char const *file,
    int line,
    char const *expr,
    char const *message,
    ...);

#define CHECK_VA(cond, ...) \
  do { \
    if (!(cond)) { \
      testing_fail(__func__, __FILE__, __LINE__, #cond, __VA_ARGS__); \
      return; \
    } \
  } while (0);

#define CHECK(cond) CHECK_VA(cond, NULL)

#define TESTING_REGISTER(func) testing_register(&(func), #func)

#ifdef __cplusplus
}
#endif

#endif
