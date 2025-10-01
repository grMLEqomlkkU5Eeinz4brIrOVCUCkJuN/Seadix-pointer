#include "RadixTrie.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <numeric>

RadixTrie::RadixTrie()
	: root(new RadixTrieNode()), wordCount(0), normalizationEnabled(true),
	  allowWhitespaceInTokens(false) {}

RadixTrie::~RadixTrie() { destroy(); }

void RadixTrie::destroy() {
	if (root) {
		delete root;
		root = nullptr;
	}
	wordCount = 0;
}

void RadixTrie::clear() {
	destroy();
	root = new RadixTrieNode();
	wordCount = 0;
}

size_t RadixTrie::size() const { return wordCount; }

bool RadixTrie::empty() const { return wordCount == 0; }

size_t RadixTrie::lcpLength(const std::string &a, const std::string &b) {
	size_t n = std::min(a.size(), b.size());
	size_t i = 0;
	for (; i < n && a[i] == b[i]; ++i) {
	}
	return i;
}

bool RadixTrie::insert(const std::string &rawWord) {
	std::string word = normalizeMaybe(rawWord);
	if (!allowWhitespaceInTokens) {
		// Fast path: no spaces -> treat as single token
		if (word.find(' ') == std::string::npos) {
			if (!isValidToken(word))
				return false;
			RadixTrieNode *node = root;
			size_t pos = 0;
			while (pos < word.size()) {
				char firstChar = word[pos];
				RadixTrieNode *child = node->getChild(firstChar);
				if (!child) {
					std::string suffix = word.substr(pos);
					RadixTrieNode *newNode = new RadixTrieNode(suffix, true);
					node->addChild(firstChar, newNode);
					wordCount++;
					return true;
				}
				size_t common = lcpLength(child->prefix, word.substr(pos));
				if (common == child->prefix.size()) {
					pos += common;
					node = child;
					continue;
				}
				std::string childSuffix = child->prefix.substr(common);
				RadixTrieNode *splitNode =
					new RadixTrieNode(child->prefix.substr(0, common), false);
				node->addChild(splitNode->prefix[0], splitNode);
				splitNode->addChild(childSuffix[0], child);
				child->prefix = childSuffix;
				std::string newSuffix = word.substr(pos + common);
				if (newSuffix.empty()) {
					splitNode->isEndOfWord = true;
				} else {
					RadixTrieNode *leaf = new RadixTrieNode(newSuffix, true);
					splitNode->addChild(newSuffix[0], leaf);
				}
				wordCount++;
				return true;
			}
			if (!node->isEndOfWord) {
				node->isEndOfWord = true;
				wordCount++;
				return true;
			}
			return false;
		}
		// Slow path: split on spaces and insert each token
		auto tokens = splitOnSpaces(word);
		bool any = false;
		for (const auto &t : tokens) {
			if (!isValidToken(t))
				continue;
			if (insert(t))
				any = true; // reuse fast single-token path
		}
		return any;
	}
	if (!isValidToken(word)) {
		return false;
	}

	RadixTrieNode *node = root;
	size_t pos = 0;
	while (pos < word.size()) {
		char firstChar = word[pos];
		RadixTrieNode *child = node->getChild(firstChar);
		if (!child) {
			std::string suffix = word.substr(pos);
			RadixTrieNode *newNode = new RadixTrieNode(suffix, true);
			node->addChild(firstChar, newNode);
			wordCount++;
			return true;
		}

		size_t common = lcpLength(child->prefix, word.substr(pos));
		if (common == child->prefix.size()) {
			pos += common;
			node = child;
			continue;
		}

		// split child
		std::string childSuffix = child->prefix.substr(common);
		RadixTrieNode *splitNode =
			new RadixTrieNode(child->prefix.substr(0, common), false);
		// rewire
		node->addChild(splitNode->prefix[0], splitNode);
		splitNode->addChild(childSuffix[0], child);
		child->prefix = childSuffix;

		// add new leaf for the remaining of the word
		std::string newSuffix = word.substr(pos + common);
		if (newSuffix.empty()) {
			splitNode->isEndOfWord = true;
		} else {
			RadixTrieNode *leaf = new RadixTrieNode(newSuffix, true);
			splitNode->addChild(newSuffix[0], leaf);
		}
		wordCount++;
		return true;
	}

	// exact match path ended inside a node
	if (!node->isEndOfWord) {
		node->isEndOfWord = true;
		wordCount++;
		return true;
	}
	return false; // already existed
}

bool RadixTrie::search(const std::string &rawWord) const {
	std::string word = normalizeMaybe(rawWord);
	if (!allowWhitespaceInTokens) {
		if (word.find(' ') == std::string::npos) {
			if (!isValidToken(word))
				return false;
			const RadixTrieNode *node = root;
			size_t pos = 0;
			while (pos < word.size()) {
				char firstChar = word[pos];
				const RadixTrieNode *child = node->getChild(firstChar);
				if (!child)
					return false;
				const std::string &p = child->prefix;
				if (word.compare(pos, p.size(), p) != 0)
					return false;
				pos += p.size();
				node = child;
			}
			return node->isEndOfWord;
		}
		auto tokens = splitOnSpaces(word);
		if (tokens.empty())
			return false;
		for (const auto &t : tokens) {
			if (!search(t))
				return false; // reuse fast path
		}
		return true;
	}
	if (!isValidToken(word)) {
		return false;
	}
	const RadixTrieNode *node = root;
	size_t pos = 0;
	while (pos < word.size()) {
		char firstChar = word[pos];
		const RadixTrieNode *child = node->getChild(firstChar);
		if (!child)
			return false;
		const std::string &p = child->prefix;
		if (word.compare(pos, p.size(), p) != 0)
			return false;
		pos += p.size();
		node = child;
	}
	return node->isEndOfWord;
}

bool RadixTrie::startsWith(const std::string &rawPrefix) const {
	std::string prefix = normalizeMaybe(rawPrefix);
	// Empty prefix matches everything
	if (prefix.empty())
		return true;
	const RadixTrieNode *node = root;
	size_t pos = 0;
	while (pos < prefix.size()) {
		char firstChar = prefix[pos];
		const RadixTrieNode *child = node->getChild(firstChar);
		if (!child)
			return false;
		const std::string &p = child->prefix;
		size_t common = lcpLength(p, prefix.substr(pos));
		if (common == 0)
			return false;
		pos += common;
		if (pos == prefix.size())
			return true; // prefix ends within this edge or exactly at node
		if (common < p.size())
			return false; // diverged before finishing the edge
		node = child;
	}
	return true;
}

void RadixTrie::collectAllWords(const RadixTrieNode *node,
								const std::string &prefix,
								std::vector<std::string> &out) const {
	if (node->isEndOfWord)
		out.push_back(prefix);
	for (const auto &kv : node->children) {
		const RadixTrieNode *child = kv.second;
		collectAllWords(child, prefix + child->prefix, out);
	}
}

std::vector<std::string>
RadixTrie::getWordsWithPrefix(const std::string &rawPrefix) const {
	std::string prefix = normalizeMaybe(rawPrefix);
	std::vector<std::string> res;
	// Empty prefix returns all words
	if (prefix.empty()) {
		collectAllWords(root, "", res);
		return res;
	}
	const RadixTrieNode *node = root;
	size_t pos = 0;
	std::string accumulated;
	while (pos < prefix.size()) {
		char firstChar = prefix[pos];
		const RadixTrieNode *child = node->getChild(firstChar);
		if (!child)
			return res;
		const std::string &p = child->prefix;
		size_t common = lcpLength(p, prefix.substr(pos));
		if (common == 0)
			return res;
		pos += common;
		accumulated += p;
		node = child;
		if (pos == prefix.size())
			break; // matched fully (possibly mid-edge), start collecting from
				   // here
		if (common < p.size())
			return res; // diverged before finishing edge and without consuming
						// all prefix
	}
	collectAllWords(node, accumulated, res);
	return res;
}

bool RadixTrie::remove(const std::string &rawWord) {
	std::string word = normalizeMaybe(rawWord);
	if (!allowWhitespaceInTokens) {
		if (word.find(' ') == std::string::npos) {
			if (!isValidToken(word))
				return false;
			bool removed = removeInternal(root, word, 0);
			if (removed)
				wordCount--;
			return removed;
		}
		auto tokens = splitOnSpaces(word);
		if (tokens.empty())
			return false;
		bool all = true;
		for (const auto &t : tokens) {
			bool removed = remove(t);
			all = all && removed;
		}
		return all;
	}
	if (!isValidToken(word)) {
		return false;
	}
	bool removed = removeInternal(root, word, 0);
	if (removed)
		wordCount--;
	return removed;
}

bool RadixTrie::removeInternal(RadixTrieNode *node, const std::string &word,
							   size_t index) {
	if (index == word.size()) {
		if (!node->isEndOfWord)
			return false;
		node->isEndOfWord = false;
		return true;
	}
	char c = word[index];
	RadixTrieNode *child = node->getChild(c);
	if (!child)
		return false;
	const std::string &p = child->prefix;
	if (word.compare(index, p.size(), p) != 0)
		return false;
	bool removed = removeInternal(child, word, index + p.size());
	if (!removed)
		return false;
	// cleanup: if child has no children and is not end, delete it
	if (!child->isEndOfWord && !child->hasChildren()) {
		node->removeChild(p[0]);
		delete child;
	} else if (child->getChildrenCount() == 1 && !child->isEndOfWord) {
		// merge with single child to keep radix compact
		auto it = child->children.begin();
		RadixTrieNode *grand = it->second;
		child->prefix += grand->prefix;
		child->isEndOfWord = grand->isEndOfWord;
		child->children = std::move(grand->children);
		delete grand;
	}
	return true;
}

std::vector<bool>
RadixTrie::batchInsert(const std::vector<std::string> &words) {
	std::vector<bool> res;
	res.reserve(words.size());
	for (const auto &w : words)
		res.push_back(insert(w));
	return res;
}

std::vector<bool>
RadixTrie::batchRemove(const std::vector<std::string> &words) {
	std::vector<bool> res;
	res.reserve(words.size());
	for (const auto &w : words)
		res.push_back(remove(w));
	return res;
}

std::vector<bool>
RadixTrie::batchSearch(const std::vector<std::string> &words) const {
	std::vector<bool> res;
	res.reserve(words.size());
	for (const auto &w : words)
		res.push_back(search(w));
	return res;
}

std::string RadixTrie::normalizeToken(const std::string &s) {
	std::string out;
	out.reserve(s.size());
	for (char ch : s) {
		if (std::isalpha(static_cast<unsigned char>(ch))) {
			out.push_back(static_cast<char>(
				std::tolower(static_cast<unsigned char>(ch))));
		} else if (ch == '\'' || ch == '-') {
			out.push_back(ch);
		} else if (std::isspace(static_cast<unsigned char>(ch))) {
			if (!out.empty() && out.back() != ' ')
				out.push_back(
					' '); // collapse whitespace into single space within token
		}
		// ignore other characters
	}
	// trim trailing space if any
	if (!out.empty() && out.back() == ' ')
		out.pop_back();
	return out;
}

bool RadixTrie::isValidToken(const std::string &s) { return !s.empty(); }

void RadixTrie::setNormalizationEnabled(bool enabled) {
	normalizationEnabled = enabled;
}

std::string RadixTrie::normalizeMaybe(const std::string &s) const {
	if (!normalizationEnabled)
		return s;
	return normalizeToken(s);
}

void RadixTrie::setAllowWhitespaceInTokens(bool enabled) {
	allowWhitespaceInTokens = enabled;
}

std::vector<std::string> RadixTrie::splitOnSpaces(const std::string &s) {
	std::vector<std::string> out;
	std::string cur;
	cur.reserve(s.size());
	for (char ch : s) {
		if (ch == ' ') {
			if (!cur.empty()) {
				out.push_back(cur);
				cur.clear();
			}
		} else {
			cur.push_back(ch);
		}
	}
	if (!cur.empty())
		out.push_back(cur);
	return out;
}

std::vector<bool> RadixTrie::insertFromFile(const std::string &filePath) {
	std::vector<bool> results;
	std::ifstream file(filePath);
	if (!file.is_open())
		return results;
	std::string line;
	results.reserve(1024);
	while (std::getline(file, line)) {
		if (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
			while (!line.empty() &&
				   (line.back() == '\r' || line.back() == '\n'))
				line.pop_back();
		}
		results.push_back(insert(line));
	}
	return results;
}

std::vector<bool> RadixTrie::removeFromFile(const std::string &filePath) {
	std::vector<bool> results;
	std::ifstream file(filePath);
	if (!file.is_open())
		return results;
	std::string line;
	results.reserve(1024);
	while (std::getline(file, line)) {
		if (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
			while (!line.empty() &&
				   (line.back() == '\r' || line.back() == '\n'))
				line.pop_back();
		}
		results.push_back(remove(line));
	}
	return results;
}

std::vector<bool> RadixTrie::searchFromFile(const std::string &filePath) const {
	std::vector<bool> results;
	std::ifstream file(filePath);
	if (!file.is_open())
		return results;
	std::string line;
	results.reserve(1024);
	while (std::getline(file, line)) {
		if (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
			while (!line.empty() &&
				   (line.back() == '\r' || line.back() == '\n'))
				line.pop_back();
		}
		results.push_back(search(line));
	}
	return results;
}

size_t RadixTrie::countNodes(const RadixTrieNode *node) const {
	if (!node)
		return 0;
	size_t total = 1;
	for (const auto &kv : node->children)
		total += countNodes(kv.second);
	return total;
}

size_t RadixTrie::countChildrenPointers(const RadixTrieNode *node) const {
	if (!node)
		return 0;
	size_t total = node->children.size();
	for (const auto &kv : node->children)
		total += countChildrenPointers(kv.second);
	return total;
}

size_t RadixTrie::nodeCount() const { return countNodes(root); }

size_t RadixTrie::childrenPointersCount() const {
	return countChildrenPointers(root);
}