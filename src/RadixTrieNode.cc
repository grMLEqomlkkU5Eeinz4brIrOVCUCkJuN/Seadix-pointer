#include "RadixTrieNode.h"
#include <algorithm>
#include <sstream>

// Took me a while to decide, but i struggled with making the decision between
// using a vector and an unordered_map. i decided to use an unordered_map
// because of memory, using vectors presented a 95+% memory overhead in my
// tests. I just hope that the unordered_map implementation is efficient enough.

RadixTrieNode::RadixTrieNode() : prefix(""), isEndOfWord(false) {}
RadixTrieNode::RadixTrieNode(const std::string &prefix, bool isEndOfWord)
	: prefix(prefix), isEndOfWord(isEndOfWord) {}

RadixTrieNode::~RadixTrieNode() {
	for (auto &entry : children) {
		delete entry.second;
	}
	children.clear();
}

bool RadixTrieNode::hasChildren() const { return !children.empty(); }
size_t RadixTrieNode::getChildrenCount() const { return children.size(); }
bool RadixTrieNode::hasChild(char c) const {
	return findChildIndex(c) != static_cast<size_t>(-1);
}

RadixTrieNode *RadixTrieNode::getChild(char c) {
	size_t idx = findChildIndex(c);
	return idx != static_cast<size_t>(-1) ? children[idx].second : nullptr;
}

const RadixTrieNode *RadixTrieNode::getChild(char c) const {
	size_t idx = findChildIndex(c);
	return idx != static_cast<size_t>(-1) ? children[idx].second : nullptr;
}

void RadixTrieNode::addChild(char c, RadixTrieNode *child) {
	size_t idx = findChildIndex(c);
	if (idx != static_cast<size_t>(-1)) {
		// replace existing
		children[idx].second = child;
		return;
	}
	// insert maintaining sorted order
	auto it = std::lower_bound(children.begin(), children.end(), c,
							   [](const std::pair<char, RadixTrieNode *> &kv,
								  char key) { return kv.first < key; });
	children.insert(it, std::make_pair(c, child));
}
void RadixTrieNode::removeChild(char c) {
	size_t idx = findChildIndex(c);
	if (idx != static_cast<size_t>(-1)) {
		children.erase(children.begin() + static_cast<long>(idx));
	}
}

void RadixTrieNode::clear() {
	children.clear();
	isEndOfWord = false;
}

size_t RadixTrieNode::findChildIndex(char c) const {
	auto it = std::lower_bound(children.begin(), children.end(), c,
							   [](const std::pair<char, RadixTrieNode *> &kv,
								  char key) { return kv.first < key; });
	if (it != children.end() && it->first == c) {
		return static_cast<size_t>(it - children.begin());
	}
	return static_cast<size_t>(-1);
}