#include "testing/testing.h"

#include <stdarg.h>
#include <stdio.h>

static int s_nFailed;

void testing_fail(char const *file, int line, char const *expr, char const *message, ...) {
  fprintf(stderr, "Check failed: %s@%d: %s\n", file, line, expr);
  va_list va;
  if (message) {
    fprintf(stderr, "  ");
    va_start(va, message);
    vfprintf(stderr, message, va);
    va_end(va);
    fprintf(stderr, "\n");
  }
  s_nFailed++;
}

int testing_report() {
  if (s_nFailed) {
    fprintf(stderr, "Failed: %d check(s)\n", s_nFailed);
  } else {
    fprintf(stderr, "Tests PASSED\n");
  }
  return !!s_nFailed;
}
