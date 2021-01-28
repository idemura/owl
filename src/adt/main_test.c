#include <stdio.h>

#include "testing/testing.h"

void test_TreeMap_begin();
void test_TreeMap_put();
void test_TreeMap_end();

int main(int argc, char **argv) {
  testing_init(argc, argv);

  test_TreeMap_begin();
  TESTING_REGISTER(test_TreeMap_put);
  test_TreeMap_end();

  bool passed = testing_run();
  testing_finish();

  return passed ? 0 : 1;
}
