import * as fs from "fs";
import * as os from "os";
import * as path from "path";

// Import the addon wrapper. Tests assume `npm run build` was executed before `npm test`.
// We import from lib (TS) which exports the native addon class.
const SeaDix = require("../lib");

describe("SeaDix RadixTrie", () => {
	let trie: any;

	beforeEach(() => {
		trie = new SeaDix();
	});

	test("initial state utilities", () => {
		expect(trie.empty()).toBe(true);
		expect(trie.size()).toBe(0);
		expect(trie.nodeCount()).toBeGreaterThanOrEqual(1);
		expect(trie.childrenPointersCount()).toBeGreaterThanOrEqual(0);
	});

	test("insert, search, duplicate insert, remove", () => {
		expect(trie.insert("hello")).toBe(true);
		expect(trie.insert("world")).toBe(true);
		expect(trie.insert("hello")).toBe(false); // duplicate

		expect(trie.size()).toBe(2);
		expect(trie.empty()).toBe(false);

		expect(trie.search("hello")).toBe(true);
		expect(trie.search("world")).toBe(true);
		expect(trie.search("nope")).toBe(false);

		expect(trie.startsWith("he")).toBe(true);
		const pref = trie.wordsWithPrefix("he") as string[];
		expect(new Set(pref)).toEqual(new Set(["hello"]));

		expect(trie.remove("world")).toBe(true);
		expect(trie.remove("world")).toBe(false);
		expect(trie.search("world")).toBe(false);
		expect(trie.size()).toBe(1);
	});

	test("batch operations", () => {
		const words = ["help", "helium", "hero"];
		const inserted = trie.insertBatch(words) as boolean[];
		expect(inserted).toEqual([true, true, true]);

		const searched = trie.searchBatch(["help", "hero", "missing"]) as boolean[];
		expect(searched).toEqual([true, true, false]);

		const removed = trie.removeBatch(["hero", "missing"]) as boolean[];
		expect(removed).toEqual([true, false]);

		const remain = trie.searchBatch(words) as boolean[];
		expect(remain).toEqual([true, true, false]);
	});

	test("clear resets trie", () => {
		trie.insert("a");
		trie.insert("b");
		expect(trie.size()).toBe(2);
		trie.clear();
		expect(trie.size()).toBe(0);
		expect(trie.empty()).toBe(true);
	});

	test("file streaming insert/search/remove", () => {
		const tmpDir = fs.mkdtempSync(path.join(os.tmpdir(), "seadix-"));
		const filePath = path.join(tmpDir, "words.txt");
		const lines = ["alpha", "beta", "gamma"];
		fs.writeFileSync(filePath, lines.join("\n"), "utf8");

		const ins = trie.insertFromFile(filePath) as boolean[];
		expect(ins).toEqual([true, true, true]);

		const sea = trie.searchFromFile(filePath) as boolean[];
		expect(sea).toEqual([true, true, true]);

		const rem = trie.removeFromFile(filePath) as boolean[];
		expect(rem).toEqual([true, true, true]);

		const seaAfter = trie.searchFromFile(filePath) as boolean[];
		expect(seaAfter).toEqual([false, false, false]);

		// cleanup
		fs.unlinkSync(filePath);
		fs.rmdirSync(tmpDir);
	});

	test("unicode accents and normalization (NFC/NFD)", () => {
		// café (NFC) and cafe + combining acute (NFD)
		const cafePlain = "cafe";
		const cafeNFC = "caf\u00E9"; // café
		const cafeNFD = "cafe\u0301"; // e + COMBINING ACUTE

		// Insert accented forms; internal normalization currently strips non-ASCII letters
		expect(trie.insert(cafeNFC)).toBe(true);
		expect(trie.search(cafeNFC)).toBe(true);
		// After normalization, "café" becomes "caf"; searching "cafe" should be false
		expect(trie.search(cafePlain)).toBe(false);
		// But the base prefix should still work
		expect(trie.startsWith("caf")).toBe(true);
		let words = trie.wordsWithPrefix("caf") as string[];
		expect(words).toContain("caf");

		// Insert NFD form; it should normalize similarly
		expect(trie.insert(cafeNFD)).toBe(true);
		expect(trie.search(cafeNFD)).toBe(true);
		// words with prefix still return normalized tokens
		words = trie.wordsWithPrefix("caf") as string[];
		expect(new Set(words)).toContain("caf");

		// Insert plain ASCII "cafe" and ensure lookups behave as expected
		// This is now a duplicate of the NFD insertion (which normalized to "cafe")
		expect(trie.insert(cafePlain)).toBe(false);
		expect(trie.search(cafePlain)).toBe(true);
		// Accented queries still map to their normalized equivalents
		expect(trie.search(cafeNFC)).toBe(true);
		expect(trie.search(cafeNFD)).toBe(true);
	});

	 test("multi-word tokens treated as single token", () => {
		trie.setAllowWhitespaceInTokens(true);
		const phrase = "foo    bar"; // multiple spaces collapse
		const trimmed = "foo bar";
		// insert should succeed once, duplicate returns false
		expect(trie.insert(phrase)).toBe(true);
		expect(trie.insert(trimmed)).toBe(false);
		// search matches normalized form
		expect(trie.search(phrase)).toBe(true);
		expect(trie.search(trimmed)).toBe(true);
		// startsWith should handle prefixes across space boundary
		expect(trie.startsWith("foo ")).toBe(true);
		expect(trie.startsWith("foo b")).toBe(true);
		// wordsWithPrefix returns full token
		const list = trie.wordsWithPrefix("foo") as string[];
		expect(list).toContain("foo bar");
		expect(list).not.toContain("foo");
		// remove clears the phrase
		expect(trie.remove(phrase)).toBe(true);
		expect(trie.search(trimmed)).toBe(false);
	});
});


