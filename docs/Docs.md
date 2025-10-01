Design and API
==============

Overview
--------
The radix trie stores strings by compressing common prefixes. Nodes hold edge labels (substrings) and a flag for end-of-word. The Node.js layer exposes methods via N-API.

Implementation
--------------
- Core: `src/RadixTrie.*`
  - Insert splits edges on partial matches and merges after deletions when possible.
  - Search walks edges by matching substrings.
  - StartsWith returns true if the prefix matches up to any point on an edge.
  - Batch methods iterate over vectors and reuse single-item logic.
  - File methods stream with `std::ifstream` and process one line per token.
  - Node children are stored in a sorted vector of `(char, RadixTrieNode*)` for cache-friendly scans.

Normalization
-------------
- Lowercase alphabetic characters are kept.
- `'` and `-` are allowed.
- Whitespace handling is configurable:
  - If `allowWhitespaceInTokens = false` (default), whitespace splits input into separate tokens.
  - If `allowWhitespaceInTokens = true`, spaces are allowed inside tokens (e.g., "foo bar").
- Empty tokens are ignored.

Performance and memory trade-offs
---------------------------------
The configuration flags impact ingestion throughput and memory usage. Query and removal operations are effectively constant-time at the scale of our benchmarks (≈ 0.09–0.18 ms per batch file), with negligible memory impact.

- Allowing whitespace inside tokens (`allowWhitespaceInTokens = true`):
  - Typically increases ingestion throughput on term-like datasets (e.g., ≈ 7.0 s → ≈ 4.8 s for a 101.8 MB terms file).
  - Significantly increases memory usage (e.g., RSS delta ≈ 223 MB → ≈ 856 MB on the same file).

- Disabling normalization (`setNormalization(false)` or `NO_NORMALIZATION=1`):
  - Modest ingestion speedup when whitespace is disabled.
  - Similar memory usage to default in that scenario.

Recommendations
---------------
- If memory is constrained: keep `allowWhitespaceInTokens = false`. You trade ~40–50% slower ingestion for ~3–4× lower memory on term datasets; search/remove remain just as fast.
- If ingestion speed matters more and RAM is ample: enable `allowWhitespaceInTokens`.
- Decide normalization based on correctness needs (case/diacritics); performance impact is comparatively small.

See also: `docs/results/terms_all.txt` and `docs/Results.md` for raw numbers and summaries.

N-API wrapper
-------------
`SeaDix` (class)
- `insert(word: string): boolean`
- `search(word: string): boolean`
- `remove(word: string): boolean`
- `startsWith(prefix: string): boolean`
- `wordsWithPrefix(prefix: string): string[]`
- `insertBatch(words: string[]): boolean[]`
- `searchBatch(words: string[]): boolean[]`
- `removeBatch(words: string[]): boolean[]`
- `insertFromFile(filePath: string): boolean[]`
- `searchFromFile(filePath: string): boolean[]`
- `removeFromFile(filePath: string): boolean[]`
- `clear(): void`
- `size(): number`
- `empty(): boolean`
- `nodeCount(): number`
- `childrenPointersCount(): number`
 - `setNormalization(enabled: boolean): void`
 - `setAllowWhitespaceInTokens(enabled: boolean): void`

Benchmarks
----------
- Harness uses high-resolution timers.
- File benchmarks measure full-file iterations and print files/s, lines/s, and per-iteration RSS deltas.
- Env flags:
  - `NO_NORMALIZATION=1` to disable normalization.
  - `ALLOW_WHITESPACE=1` to allow whitespace inside tokens.

Notes
-----
- Object conversion between JS and C++ is avoided in hot loops; file streaming is implemented in C++.
- The library focuses on clarity and measurable behavior.
- Expect changes; the project is experimental.


