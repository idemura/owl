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
      testingFail(__func__, __FILE__, __LINE__, #cond, __VA_ARGS__); \
      return; \
    } \
  } while (0);

#define CHECK(cond) CHECK_VA(cond, NULL)

#define TEST_CASE(func) (TestCase) {.body=&(func), #func}

typedef struct {
  void (*body)(void);
  const char *name;
} TestCase;

void testingInit(int argc, char **argv);
void testingAdd(void (*begin)(void), void (*end)(void), TestCase *tests, size_t testsNum);
bool testingRun(void);
void testingFinish(void);

/* Internal */
void testingFail(
    const char *func, const char *file, int line, const char *expr, const char *message, ...);

#endif
