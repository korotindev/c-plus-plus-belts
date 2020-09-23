#include "tests.h"
#include "utils/test_runner.h"

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestIntegrationTest1);
    RUN_TEST(tr, TestIntegrationTest2);
    RUN_TEST(tr, TestIntegrationTest3);
    RUN_TEST(tr, TestIntegrationTest4);
    return 0;
}