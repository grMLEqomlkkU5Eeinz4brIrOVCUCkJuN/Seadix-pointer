#ifndef RADIX_TRIE_NODE_H
#define RADIX_TRIE_NODE_H
#include <string>
#include <vector>

class RadixTrieNode {
  public:
	RadixTrieNode();
	RadixTrieNode(const std::string &prefix, bool isEndOfWord = false);
	~RadixTrieNode();

	std::string prefix;
	bool isEndOfWord;
	// Sorted by key for cache-friendly scans and predictable iteration
	std::vector<std::pair<char, RadixTrieNode *>> children;

	bool hasChildren() const;
	size_t getChildrenCount() const;
	bool hasChild(char c) const;
	RadixTrieNode *getChild(char c);
	const RadixTrieNode *getChild(char c) const;
	void addChild(char c, RadixTrieNode *child);
	void removeChild(char c);

	void clear();

  private:
	size_t findChildIndex(char c) const;
};

#endif // RADIX_TRIE_NODE_H