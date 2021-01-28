#ifndef TESTING_TESTING_H_
#define TESTING_TESTING_H_

#include <stddef.h>

#include "types/primitive.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*TestCaseFP)();

void testing_fail(char const *file, int line, char const *expr, char const *message, ...);
int testing_report();

#define CHECK_VA(cond, ...) \
  do { \
    if (!(cond)) { \
      testing_fail(__FILE__, __LINE__, #cond, __VA_ARGS__); \
      return; \
    } \
  } while (0);

#define CHECK(cond) CHECK_VA(cond, NULL)

#ifdef __cplusplus
}
#endif

#endif
