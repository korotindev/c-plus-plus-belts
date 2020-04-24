#include "test_runner.h"
#include "profile.h"
#include <iostream>

void TestFunc() {
  std::cout << "Hello, world!" << endl;
}

int main() {
    TestRunner tr;

    {
        LOG_DURATION("test message")
        int slowComputation = 5 + 3;
        std::cout << "slowComputation = " << slowComputation << endl;
    }

    RUN_TEST(tr, TestFunc);
    return 0;
}
