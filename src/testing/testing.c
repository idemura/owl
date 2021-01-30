#include "testing/testing.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_MAX 1200

static const char kTestPrefix[] = "test_";

typedef struct {
  void (*begin)(void);
  void (*end)(void);
  TestCase *tests;
  size_t testsNum;
} RegEntry;

static int nFailedChecks;
static int regSize;
static RegEntry reg[TEST_MAX];

void testingInit(int argc, char **argv) {
  // Empty
}

void testingAdd(void (*begin)(void), void (*end)(void), TestCase *tests, size_t testsNum) {
  if (regSize == TEST_MAX) {
    fprintf(stderr, "Test case registry memory exceeded\n");
    exit(1);
  }
  reg[regSize++] = (RegEntry) {begin, end, tests, testsNum};
}

bool testingRun(void) {
  size_t prefixLen = strlen(kTestPrefix);

  int nFailedTests = 0;
  int nTests = 0;
  for (int i = 0; i < regSize; i++) {
    RegEntry *entry = &reg[i];
    if (entry->begin) {
      entry->begin();
    }
    for (int j = 0; j < entry->testsNum; j++) {
      int f = nFailedChecks;
      entry->tests[j].body();
      const char *name = entry->tests[j].name;
      if (strncmp(name, kTestPrefix, prefixLen) == 0) {
        name += prefixLen;
      }
      if (nFailedChecks > f) {
        fprintf(stderr, "FAILED %s\n", name);
        nFailedTests++;
      } else {
        fprintf(stderr, "Passed %s\n", name);
      }
      nTests++;
    }
    if (entry->end) {
      entry->end();
    }
  }
  if (nFailedTests) {
    fprintf(stderr, "Failed: %u/%u tests\n", nFailedTests, nTests);
  } else {
    fprintf(stderr, "All tests PASSED\n");
  }
  return nFailedTests > 0;
}

void testingFinish(void) {
  // Empty. Release memory.
}

void testingFail(
    const char *func, const char *file, int line, const char *expr, const char *message, ...) {
  fprintf(stderr, "Check failed in %s %s@%d: %s\n", func, file, line, expr);
  va_list va;
  if (message) {
    fprintf(stderr, "  ");
    va_start(va, message);
    vfprintf(stderr, message, va);
    va_end(va);
    fprintf(stderr, "\n");
  }
  nFailedChecks++;
}
