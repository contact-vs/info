/**
 * https://leetcode.com/problems/two-sum-iv-input-is-a-bst/
 * Given the root of a Binary Search Tree and a target number k,
 * return true if there exist two elements in the BST
 * such that their sum is equal to the given target.
 * Input: root = [5,3,6,2,4,null,7], k = 9 Output: true
 * Input: root = [5,3,6,2,4,null,7], k = 28 Output: false
 * Constraints: The number of contextNodes in the tree is in the range [1, 10^4]. -10^4 <= Node.val <= 10^4
 * root is guaranteed to be a valid binary search tree. -10^5 <= k <= 10^5
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     TreeNode *left;
 *     TreeNode *right;
 *     TreeNode() : val(0), left(nullptr), right(nullptr) {}
 *     TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
 *     TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
 * };
 */
#include <iterator>
#include <vector>
#include <iostream>
struct TreeNode {
	int val;
	TreeNode* left;
	TreeNode* right;
	TreeNode() : val(0), left(nullptr), right(nullptr) {}
	explicit TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
	TreeNode(int x, TreeNode* left, TreeNode* right) : val(x), left(left), right(right) {}
};

inline TreeNode* left(TreeNode* t) { return t->left; }
inline TreeNode* right(TreeNode* t) { return t->right; }

template<class T_TreeNode>
class BinarySearchTreeIterator {
private:
	bool reverse;
	std::vector<T_TreeNode*> contextNodes{};
	std::vector<bool> visitedMainBranch{};
	std::vector<bool> visitedOtherBranch{};
	BinarySearchTreeIterator(T_TreeNode* data, bool reverse_) : reverse(reverse_) {
		downTo(data);
	}
public:
	using iterator_category = std::forward_iterator_tag;
	using difference_type = std::ptrdiff_t;
	using value_type = T_TreeNode;
	using pointer = T_TreeNode*;
	using reference = T_TreeNode&;
	using Iterator = BinarySearchTreeIterator;
	static BinarySearchTreeIterator of(T_TreeNode* data) {
		return BinarySearchTreeIterator(data, false);
	}
	static BinarySearchTreeIterator reverse_of(T_TreeNode* data) {
		return BinarySearchTreeIterator(data, true);
	}
	reference operator*() const { return *data(); }
	pointer operator->() { return data(); }
	friend bool operator== (const Iterator& a, const Iterator& b) { return a.data() == b.data(); };
	friend bool operator!= (const Iterator& a, const Iterator& b) { return !operator==(a, b); };
	Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
	Iterator& operator++() {
		next();
		return *this;
	}
private:
	void downTo(T_TreeNode* data_) {
		contextNodes.push_back(data_);
		visitedMainBranch.push_back(false);
		visitedOtherBranch.push_back(false);
		while (mainBranch()) {
			visitedMainBranch.back() = true;
			downTo(mainBranch());
		}
	}
	pointer data() const {
		return contextNodes.back();
	}
	pointer mainBranch() const {
		return !reverse ? left(data()) : right(data());
	}
	pointer otherBranch() const {
		return reverse ? left(data()) : right(data());
	}
	void next() {
		if (otherBranch() && !visitedOtherBranch.back()) {
			visitedOtherBranch.back() = true;
			downTo(otherBranch());
		}
		else {
			if (contextNodes.empty()) return;
			contextNodes.pop_back();
			visitedMainBranch.pop_back();
			visitedOtherBranch.pop_back();
		}
	}
};

class Solution {
public:
	bool findTarget(TreeNode* root, int k) const {
		auto left = BinarySearchTreeIterator<TreeNode>::of(root);
		auto right = BinarySearchTreeIterator<TreeNode>::reverse_of(root);
		while (left != right) {
			int sum = left->val + right->val;
			if (sum == k) return true;
			sum < k ? ++left : ++right;
		}
		return false;
	}

};
int main() {
	auto a2 = std::make_unique<TreeNode>(2);
	auto a4 = std::make_unique<TreeNode>(4);
	auto a3 = std::make_unique<TreeNode>(3, a2.get(), a4.get());
	auto a7 = std::make_unique<TreeNode>(7);
	auto a6 = std::make_unique<TreeNode>(6, nullptr, a7.get());
	auto a5 = std::make_unique<TreeNode>(5, a3.get(), a6.get());
	Solution s;
	std::cout << std::boolalpha << s.findTarget(a5.get(), 9) << std::endl;
	std::cout << std::boolalpha << s.findTarget(a5.get(), 28) << std::endl;
	return 0;
}
