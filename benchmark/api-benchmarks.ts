// benchmark runner
import { ensureGcExposed, printRuntimeContext, runAndReport } from "./harness";

// Import TS wrapper that re-exports the native addon class
const SeaDix = require("../lib");

async function main(): Promise<void> {
	printRuntimeContext();
	ensureGcExposed();

	const trie = new SeaDix();
	const iterations = Number(process.env.ITERATIONS ?? 500);
	const withGc = String(process.env.LOCK_GC ?? "1") === "1";

	console.log("\nNumber of iterations: ", iterations);
	console.log("\n\n\nRound 1: insert unique word");
	await runAndReport(
		"insert unique word",
		() => {
			const word = `w_${Math.random().toString(36).slice(2)}`;
			trie.remove(word);
			const ok = trie.insert(word);
			if (!ok) throw new Error("insert failed");
			trie.remove(word);
		},
		{ iterations, lockGc: withGc },
	);

	console.log("\nRound 2: search existing word");
	await runAndReport(
		"search existing word",
		() => {
			const word = "persist_word";
			trie.insert(word);
			const ok = trie.search(word);
			if (!ok) throw new Error("search should be true");
		},
		{ iterations, lockGc: withGc },
	);

	console.log("\nRound 3: search missing word");
	await runAndReport(
		"search missing word",
		() => {
			const word = "missing_word";
			trie.remove(word);
			const ok = trie.search(word);
			if (ok) throw new Error("search should be false");
		},
		{ iterations, lockGc: withGc },
	);

	console.log("\nRound 4: startsWith existing prefix");
	await runAndReport(
		"startsWith existing prefix",
		() => {
			const pref = "pre_";
			trie.insert(pref + "a");
			const ok = trie.startsWith(pref);
			if (!ok) throw new Error("startsWith should be true");
		},
		{ iterations, lockGc: withGc },
	);

	console.log("\nRound 5: wordsWithPrefix small set");
	await runAndReport(
		"wordsWithPrefix small set",
		() => {
			const pref = "wxp_";
			trie.insert(pref + "1");
			trie.insert(pref + "2");
			const arr = trie.wordsWithPrefix(pref);
			if (!Array.isArray(arr) || arr.length < 1) throw new Error("empty");
		},
		{ iterations: Math.max(200, Math.floor(iterations / 5)), lockGc: withGc },
	);

	console.log("\nRound 6: insertBatch small");
	await runAndReport(
		"insertBatch small",
		() => {
			const base = `ib_${Math.random().toString(36).slice(2)}`;
			const words = [base + "a", base + "b", base + "c", base + "d"];
			trie.removeBatch(words);
			const res = trie.insertBatch(words);
			if (!Array.isArray(res) || res.some((v: boolean) => !v))
				throw new Error("insertBatch failure");
			trie.removeBatch(words);
		},
		{ iterations, lockGc: withGc },
	);

	console.log("\nRound 7: searchBatch small");
	await runAndReport(
		"searchBatch small",
		() => {
			const base = `sb_${Math.random().toString(36).slice(2)}`;
			const words = [base + "a", base + "b", base + "c", base + "d"];
			trie.insertBatch(words);
			const res = trie.searchBatch(words);
			if (!Array.isArray(res) || res.some((v: boolean) => !v))
				throw new Error("searchBatch unexpected");
			trie.removeBatch(words);
		},
		{ iterations, lockGc: withGc },
	);

	console.log("\nRound 8: removeBatch small");
	await runAndReport(
		"removeBatch small",
		() => {
			const base = `rb_${Math.random().toString(36).slice(2)}`;
			const words = [base + "a", base + "b", base + "c", base + "d"];
			trie.insertBatch(words);
			const res = trie.removeBatch(words);
			if (!Array.isArray(res) || res.some((v: boolean) => !v))
				throw new Error("removeBatch unexpected");
		},
		{ iterations, lockGc: withGc },
	);

	console.log("\nRound 9: size/empty utilities");
	await runAndReport(
		"size/empty utilities",
		() => {
			void trie.size();
			void trie.empty();
			void trie.nodeCount();
			void trie.childrenPointersCount();
		},
		{ iterations: iterations * 2, lockGc: withGc },
	);
}

main().catch((err) => {
	console.error(err);
	process.exit(1);
});


