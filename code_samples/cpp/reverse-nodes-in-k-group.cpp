#include <iostream>
using namespace std;
/* https://leetcode.com/problems/reverse-nodes-in-k-group/
25. Reverse Nodes in k-Group Hard
Given the head of a linked list
, reverse the nodes of the list k at a time
, and return the modified list.
k is a positive integer and is less than or equal to the length of the linked list.
If the number of nodes is not a multiple of k
then left-out nodes, in the end, should remain as it is.
You may not alter the values in the list's nodes
, only nodes themselves may be changed.

Input: head = [1,2,3,4,5], k = 2
Output: [2,1,4,3,5]
Input: head = [1,2,3,4,5], k = 3
Output: [3,2,1,4,5]
*/
struct ListNode
{
    int val;
    ListNode* next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode* next) : val(x), next(next) {}
};
void printList(ListNode* head)
{
    cout << "[";
    for (; head; head = head->next)
    {
        cout << head->val << " ";
    }
    cout << "]" << endl;
}
class Solution
{
public:
    ListNode* reverseKGroup(ListNode* head, int k)
    {
        if (k < 2 || head == nullptr)
            return head;
        int groupLinks = k - 1;
        return reverseKGroupSub(head, groupLinks);
    }

private:
    ListNode* reverseKGroupSub(ListNode* head, int links)
    {
        ListNode* ret = head;
        ListNode* prevGroupLast = nullptr;
        int n;
        while (head)
        {
            n = links;
            ListNode* saveHead = head;
            ListNode* groupHead = head;
            head = reverse1Group(groupHead, n);
            if (n)
            {
                n = links - n;
                reverse1Group(groupHead, n);
            }
            if (!prevGroupLast)
            {
                ret = groupHead; // only first time;
            }
            else
            {
                prevGroupLast->next = groupHead;
            }
            prevGroupLast = saveHead;
        }
        return ret;
    }
    ListNode* reverse1Group(ListNode*& head, int& links)
    {
        if (!head || links < 1)
            return head;
        ListNode* last = head;
        ListNode* next = head->next;
        while (next && links)
        {
            ListNode* far = next->next;
            next->next = head;
            head = next;
            next = far;
            --links;
        }
        last->next = nullptr;
        return next;
    }
};
using LN = ListNode;
int main()
{
    auto head = new LN(1, new LN(2, new LN(3, new LN(4, new LN(5, new LN(6, new LN(7)))))));
    // auto head = new LN(1,new LN(2, new LN(3, new LN(4, new LN(5, new LN(6))))));
    // auto head = new LN(1, new LN(2));
    Solution s;
    head = s.reverseKGroup(head, 2);
    printList(head);
}
