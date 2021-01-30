#include "testing/testing.h"

void test_TreeMap(void);

int main(int argc, char **argv) {
  testingInit(argc, argv);

  test_TreeMap();

  bool passed = testingRun();
  testingFinish();

  return passed ? 0 : 1;
}
