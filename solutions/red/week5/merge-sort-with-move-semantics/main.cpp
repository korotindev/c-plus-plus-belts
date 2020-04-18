#include "test_runner.h"
#include <algorithm>
#include <memory>
#include <vector>

using namespace std;

template<typename RandomIt>
void MergeSort(RandomIt range_begin, RandomIt range_end) {
    int range_length = range_end - range_begin;
    if (range_length < 2) {
        return;
    }

    vector<typename RandomIt::value_type> elements(
            make_move_iterator(range_begin),
            make_move_iterator(range_end)
    );

    auto count = (range_end - range_begin) / 3;
    RandomIt secondPartBegin = elements.begin() + count;
    RandomIt secondPartEnd = secondPartBegin + count;

    MergeSort(elements.begin(), secondPartBegin);
    MergeSort(secondPartBegin, secondPartEnd);
    MergeSort(secondPartEnd, elements.end());

    vector<typename RandomIt::value_type> tmp;
    merge(make_move_iterator(elements.begin()), make_move_iterator(secondPartBegin), make_move_iterator(secondPartBegin), make_move_iterator(secondPartEnd), back_inserter(tmp));
    merge(make_move_iterator(tmp.begin()), make_move_iterator(tmp.end()), make_move_iterator(secondPartEnd), make_move_iterator(elements.end()),  range_begin);
}

void TestIntVector() {
    vector<int> numbers = {6, 1, 3, 9, 1, 9, 8, 12, 1};
    MergeSort(begin(numbers), end(numbers));
    ASSERT(is_sorted(begin(numbers), end(numbers)));
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestIntVector);
    return 0;
}