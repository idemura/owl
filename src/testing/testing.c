#include "testing/testing.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// (sizeof(RegEntry) * kBlockSize) is a little less than 1024
const int kBlockSize = 40;
const int kNumBlocks = 1024;
const char kTestPrefix[] = "test_";

typedef struct {
  TestCaseFP func;
  const char *funcName;
  bool failed;
} RegEntry;

static int nFailedChecks;
static int regSize;
static RegEntry *reg[kNumBlocks];

void testing_init(int argc, char **argv) {
  // Empty
}

void testing_register(TestCaseFP func, const char *funcName) {
  if (regSize == kBlockSize * kNumBlocks) {
    fprintf(stderr, "Test case registry memory exceeded\n");
    exit(1);
  }
  int q = regSize / kBlockSize;
  int r = regSize % kBlockSize;
  if (r == 0) {
    reg[q] = calloc(kBlockSize, sizeof(RegEntry));
  }

  RegEntry *entry = reg[q] + r;
  entry->func = func;
  entry->funcName = funcName;
  regSize++;
}

bool testing_run(void) {
  size_t prefixLen = strlen(kTestPrefix);

  int nFailedTests = 0;
  int nTests = 0;
  for (int b = 0, i = 0; b < regSize; b += kBlockSize, i++) {
    int n = regSize - b > kBlockSize ? kBlockSize : regSize - b;
    for (int j = 0; j < n; j++) {
      int f = nFailedChecks;
      reg[i][j].func();
      const char *printName = reg[i][j].funcName;
      if (strncmp(reg[i][j].funcName, kTestPrefix, prefixLen) == 0) {
        printName += prefixLen;
      }
      if (nFailedChecks > f) {
        reg[i][j].failed = true;
        fprintf(stderr, "FAILED %s\n", printName);
        nFailedTests++;
      } else {
        fprintf(stderr, "Passed %s\n", printName);
      }
      nTests++;
    }
  }
  if (nFailedTests) {
    fprintf(stderr, "Failed: %u/%u tests\n", nFailedTests, nTests);
  } else {
    fprintf(stderr, "All tests PASSED\n");
  }
  return nFailedTests > 0;
}

void testing_finish(void) {
  for (int i = 0; i < kNumBlocks; i++) {
    if (reg[i]) {
      free(reg[i]);
    } else {
      break;
    }
  }
}

void testing_fail(
    const char *funcName, const char *file, int line, const char *expr, const char *message, ...) {
  fprintf(stderr, "Check failed in %s %s@%d: %s\n", funcName, file, line, expr);
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
