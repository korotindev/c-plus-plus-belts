#pragma once

#include <algorithm>

using namespace std;

template<typename It>
class IteratorRange {
public:
    IteratorRange(It first, It last) : first(first), last(last) {
    }

    It begin() const {
        return first;
    }

    It end() const {
        return last;
    }

    size_t size() const {
        return last - first;
    }

private:
    It first, last;
};

template<typename Container>
auto Head(Container &c, size_t top) {
    size_t zero = 0;
    auto range_end = c.begin();
    long end_index = static_cast<long>(min(max(top, zero), c.size()));
    advance(range_end, end_index);

    return IteratorRange(begin(c), range_end);
}
