#include "testing/testing.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const size_t kNumBlocks = 1024;
const size_t kBlockSize =
    40;  // sizeof(RegEntry) * kBlockSize is a little lestt than 1024
const char kTestPrefix[] = "test_";

typedef struct {
  TestCaseFP func;
  char const *funcName;
  bool failed;
} RegEntry;

static unsigned nFailedChecks;
static RegEntry *reg[kNumBlocks];
static size_t regSize;

void testing_init(int argc, char **argv) {
  // None
}

void testing_register(TestCaseFP func, char const *funcName) {
  if (regSize == kNumBlocks * kBlockSize) {
    fprintf(stderr, "Test case registry memory exceeded\n");
    exit(1);
  }
  size_t q = regSize / kBlockSize;
  size_t r = regSize % kBlockSize;
  if (r == 0) {
    size_t blockBytes = sizeof(RegEntry) * kBlockSize;
    reg[q] = malloc(blockBytes);
    memset(reg[q], 0, blockBytes);
  }

  RegEntry *entry = reg[q] + r;
  entry->func = func;
  entry->funcName = funcName;
  regSize++;
}

bool testing_run() {
  size_t prefixLen = strlen(kTestPrefix);

  unsigned nFailedTests = 0;
  unsigned nTests = 0;
  for (size_t b = 0; b < regSize; b += kBlockSize) {
    size_t n = regSize - b > kBlockSize ? kBlockSize : regSize - b;
    for (size_t i = 0; i < n; i++) {
      unsigned f = nFailedChecks;
      reg[b][i].func();
      char const *printName = reg[b][i].funcName;
      if (strncmp(reg[b][i].funcName, kTestPrefix, prefixLen) == 0) {
        printName += prefixLen;
      }
      if (nFailedChecks > f) {
        reg[b][i].failed = true;
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

void testing_finish() {
  for (size_t i = 0; i < kNumBlocks; i++) {
    if (reg[i]) {
      free(reg[i]);
    } else {
      break;
    }
  }
}

void testing_fail(
    char const *funcName,
    char const *file,
    int line,
    char const *expr,
    char const *message,
    ...) {
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
