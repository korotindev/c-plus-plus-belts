#include "profile.h"
#include "test_runner.h"

int main() {
    {
        LOG_DURATION("test message");
    }

    ASSERT_EQUAL(1, 1);
    return 0;
}