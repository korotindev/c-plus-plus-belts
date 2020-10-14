struct ListNode {
  int val;
  ListNode* next;
  ListNode() : val(0), next(nullptr) {}
  ListNode(int x) : val(x), next(nullptr) {}
  ListNode(int x, ListNode* next) : val(x), next(next) {}
};

class Solution {
 public:
  ListNode* reverseList(ListNode* head) {
    ListNode* prev = head;
    if (head) {
      head = head->next;
      prev->next = nullptr;
    }
    while (head) {
      auto next = head->next;
      head->next = prev;
      prev = head;
      head = next;
    }

    return prev;
  }
};

int main() {
  Solution().reverseList(new ListNode(1, new ListNode(2, new ListNode(3, new ListNode(4, new ListNode(5))))));

  return 0;
}