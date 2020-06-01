#include "test_runner.h"
#include "profile.h"
#include <iostream>
#include <sstream>
#include "Request.h"

using namespace std;

void TestFunc() {
    {
        stringstream input("Stop Tolstopaltsevo: 55.611087, 37.20829");
        auto requestHandler = ParseRequest(MODIFY_TYPES_CONVERTER, input.str());
    }

    {
        stringstream input("Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye");
        auto requestHandler = ParseRequest(MODIFY_TYPES_CONVERTER, input.str());
    }

    {
        stringstream input("Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka");
        auto requestHandler = ParseRequest(MODIFY_TYPES_CONVERTER, input.str());
    }

}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestFunc);
    return 0;
}
