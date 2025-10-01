SeaDix
======

A radix trie implemented in C++ and exposed to Node.js via N-API. This repository is experimental.

What this is
-------------
- Radix trie core in C++ (`src/`)
- N-API wrapper (`src/SeaDix.*`) exposed as `SeaDix` class to Node.js
- Benchmarks for single-API calls and file-based operations (`benchmark/`)

Caveats
-------
1. Crossing the JS ↔ C++ boundary has cost. Bulk work should stay in C++ (tldr file streaming shines here).
2. File streaming happens in C++ to avoid per-line object conversions in JS.
3. `searchFromFile` and `removeFromFile` operate line-by-line, useful for exclusion rule workflows similar to WAF use cases.
4. Token normalization is simple (lowercase letters, `'` and `-`). Non-matching chars stop token accumulation at first whitespace.
5. This is not production software.

Build
-----
```bash
npm run build       # build native addon
npm run build:ts    # build TypeScript wrapper types
npm run build:all   # both
```

Test
----
```bash
npm test
```

Benchmarks
----------
File benchmarks process entire files per iteration and report files/s, lines/s, and per-iteration RSS deltas.
```bash
npm run bench:file:enable
npm run bench:file:words
npm run bench:file:terms
```

Environment flags for benchmarks
--------------------------------
- `NO_NORMALIZATION=1` disables normalization (use raw input as-is).
- `ALLOW_WHITESPACE=1` allows whitespace inside tokens (e.g., bigrams like "foo bar").

Examples:
```bash
NO_NORMALIZATION=1 npm run bench:file:terms
ALLOW_WHITESPACE=1 npm run bench:file:terms
```

Quick usage
-----------
```ts
const SeaDix = require("./lib");
const trie = new SeaDix();

// Configuration (optional)
// Enable/disable normalization (lowercasing, filtering, whitespace collapsing)
trie.setNormalization(true); // default: true
// Allow whitespace within tokens (e.g., bigrams like "foo bar")
trie.setAllowWhitespaceInTokens(false); // default: false

trie.insert("example");
trie.search("example");
trie.startsWith("exa");
trie.remove("example");

trie.insertBatch(["a", "b", "c"]);
trie.searchBatch(["a", "x"]);
trie.removeBatch(["b", "c"]);

// File operations (one token per line)
trie.insertFromFile("textfiles/enable1.txt");
trie.searchFromFile("textfiles/search.txt");
trie.removeFromFile("textfiles/remove.txt");
```

Status
------
- Experimental. More features and tests will be added.

Changes
-------
- Child container switched to a sorted vector for better cache locality.
- Added `setNormalization(enabled: boolean)` and `setAllowWhitespaceInTokens(enabled: boolean)`.
- Bench harness supports `NO_NORMALIZATION` and `ALLOW_WHITESPACE` flags.


