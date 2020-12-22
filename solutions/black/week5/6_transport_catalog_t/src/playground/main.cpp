#include <algorithm>
#include <vector>
using namespace std;

class Solution {
 public:
  vector<int> smallerNumbersThanCurrent(vector<int>& nums) {
    vector<int> sorted = [](vector<int> arr) {
      sort(arr.begin(), arr.end());
      return arr;
    }(nums);

    auto sorted_beg = sorted.begin();
    for (size_t i = 0; i < nums.size(); i++) {
      int elem = nums[i];
      auto it = lower_bound(sorted_beg, sorted.end(), elem);
      sorted[i] = distance(sorted.begin(), it);
      sorted_beg++;
    }

    return sorted;
  }
};

int main() {
  vector<int> arr = {8, 1, 2, 2, 3};
  Solution().smallerNumbersThanCurrent(arr);
  return 0;
}