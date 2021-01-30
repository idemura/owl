#include "testing/testing.h"

void test_treemap(void);

int main(int argc, char **argv)
{
    testing_init(argc, argv);

    test_treemap();

    bool passed = testing_run();
    testing_finish();

    return passed ? 0 : 1;
}
