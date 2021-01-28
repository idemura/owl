#include <stdio.h>

#include "testing/testing.h"

void TreeMap_test_begin();
void TreeMap_test_put();
void TreeMap_test_end();

int main(int argc, char **argv) {
  TreeMap_test_begin();
  TreeMap_test_put();
  TreeMap_test_end();
  return testing_report();
}
