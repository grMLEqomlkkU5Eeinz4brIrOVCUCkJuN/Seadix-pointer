#ifndef RADIX_TRIE_H
#define RADIX_TRIE_H

#include "RadixTrieNode.h"
#include <fstream>
#include <memory>
#include <string>
#include <vector>

class RadixTrie {
  public:
	RadixTrie();
	~RadixTrie();

	bool insert(const std::string &word);
	bool search(const std::string &word) const;
	bool remove(const std::string &word);
	bool startsWith(const std::string &prefix) const;
	std::vector<std::string>
	getWordsWithPrefix(const std::string &prefix) const;

	// Configuration
	void setNormalizationEnabled(bool enabled);
	void setAllowWhitespaceInTokens(bool enabled);

	// Common utilities
	void clear();
	size_t size() const; // number of words stored
	bool empty() const;

	// Memory statistics (rough estimates)
	size_t nodeCount() const;			  // number of nodes
	size_t childrenPointersCount() const; // total child pointer slots

	// Batch operations
	std::vector<bool> batchInsert(const std::vector<std::string> &words);
	std::vector<bool> batchRemove(const std::vector<std::string> &words);
	std::vector<bool> batchSearch(const std::vector<std::string> &words) const;

	// File streaming operations: one token per line
	std::vector<bool> insertFromFile(const std::string &filePath);
	std::vector<bool> removeFromFile(const std::string &filePath);
	std::vector<bool> searchFromFile(const std::string &filePath) const;

  private:
	RadixTrieNode *root;
	size_t wordCount;
	bool normalizationEnabled;
	bool allowWhitespaceInTokens;

	static size_t lcpLength(const std::string &a, const std::string &b);
	void collectAllWords(const RadixTrieNode *node, const std::string &prefix,
						 std::vector<std::string> &out) const;
	bool removeInternal(RadixTrieNode *node, const std::string &word,
						size_t index);
	size_t countNodes(const RadixTrieNode *node) const;
	size_t countChildrenPointers(const RadixTrieNode *node) const;
	void destroy();
	static std::string normalizeToken(const std::string &s);
	std::string normalizeMaybe(const std::string &s) const;
	static std::vector<std::string> splitOnSpaces(const std::string &s);
	static bool isValidToken(const std::string &s);

	RadixTrie(const RadixTrie &) = delete;
	RadixTrie &operator=(const RadixTrie &) = delete;
	RadixTrie(RadixTrie &&) = delete;
	RadixTrie &operator=(RadixTrie &&) = delete;
};

#endif // RADIX_TRIE_H