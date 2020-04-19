#include "test_runner.h"
#include <algorithm>
#include <cstdint>
#include <vector>
#include <future>
#include <numeric>

using namespace std;

template<typename Iterator>
class PaginatorRange {
public:
    PaginatorRange(Iterator range_begin, Iterator range_end) : first(range_begin), last(range_end) {}
    Iterator begin() const { return first; }
    Iterator end() const { return last; }
private:
    Iterator first, last;
};

template<typename Iterator>
class Paginator {
private:
    vector<PaginatorRange<Iterator>> ranges;
public:
    Paginator(Iterator begin, Iterator end, size_t page_size) {
        for(size_t left = distance(begin, end); left > 0;) {
            size_t range_length = min(page_size, left);
            Iterator nextRangeIterator = begin + range_length;
            ranges.push_back({begin, nextRangeIterator});

            left -= range_length;
            begin = nextRangeIterator;
        }
    }

    auto begin() const { return ranges.begin(); }
    auto end() const { return ranges.end(); }
};

template<typename RandomAccessContainer>
auto Paginate(RandomAccessContainer &container, size_t page_size) {
    return Paginator(begin(container), end(container), page_size);
}

int64_t CalculateMatrixSum(const vector<vector<int>> &matrix) {
    int64_t result = 0;
    vector<future<int>> futures;
    auto pages = Paginate(matrix, 2000);
    for (auto page : pages) {
        futures.push_back(
                async([page] {
                    int res = 0;
                    for (auto row : page) {
                        res += accumulate(row.begin(), row.end(), 0);
                    }
                    return res;
                })
        );
    }

    for (auto &f : futures) {
        result += f.get();
    }

    return result;
}

void TestCalculateMatrixSum() {
    const vector<vector<int>> matrix = {
            {1,  2,  3,  4},
            {5,  6,  7,  8},
            {9,  10, 11, 12},
            {13, 14, 15, 16}
    };
    ASSERT_EQUAL(CalculateMatrixSum(matrix), 136);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestCalculateMatrixSum);
}