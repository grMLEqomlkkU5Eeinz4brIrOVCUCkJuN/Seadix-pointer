// file benchmark (enable)
import { ensureGcExposed, printRuntimeContext, measureFunctionWithMemory, formatSample } from "./harness";
import path from "path";
import fs from "fs";

const SeaDix = require("../lib");

async function main(): Promise<void> {
	printRuntimeContext();
	ensureGcExposed();

	const trie = new SeaDix();
	// Force-enable whitespace allowance and normalization for this benchmark
	if (typeof trie.setAllowWhitespaceInTokens === "function") {
		trie.setAllowWhitespaceInTokens(true);
	}
	if (typeof trie.setNormalizationEnabled === "function") {
		trie.setNormalizationEnabled(true);
	}
	const withGc = String(process.env.LOCK_GC ?? "1") === "1";
	const iters = Number(process.env.ITERATIONS ?? 10);

	const filePath = path.resolve(__dirname, "../textfiles/enable1.txt");
	const searchFilePath = path.resolve(__dirname, "../textfiles/search.txt");
	const removeFilePath = path.resolve(__dirname, "../textfiles/remove.txt");

	console.log("\nNote: Each iteration processes the entire file once.");

	// Memory overhead report (single run, outside timed loops)
	try {
		if (typeof global.gc === "function") global.gc();
		const fileBytes = fs.statSync(filePath).size;
		const memBefore = process.memoryUsage();
		void trie.insertFromFile(filePath);
		const memAfter = process.memoryUsage();
		const rssDelta = Math.max(0, memAfter.rss - memBefore.rss);
		const ratio = fileBytes > 0 ? rssDelta / fileBytes : 0;
		console.log("\nMemory report (enable1.txt):");
		console.log(`file size: ${fileBytes} bytes`);
		console.log(`rss delta: ${rssDelta} bytes`);
		console.log(`rss/file ratio: ${ratio.toFixed(3)}`);
		trie.clear();
		if (typeof global.gc === "function") global.gc();
	} catch (e) {
		console.warn("Memory report failed:", e);
	}

	console.log("\nRound 1/3: insertFromFile enable1.txt");
	{
		const linesInEnable = fs.readFileSync(filePath, "utf8").split("\n").length;
		const s = await measureFunctionWithMemory(() => {
			void trie.insertFromFile(filePath);
			trie.clear();
		}, { iterations: iters, lockGc: withGc, warmupIterations: 1 });
		console.log(formatSample("insertFromFile enable1.txt", s));
		const filesPerSec = s.meanNs > 0 ? (1_000_000_000 / s.meanNs) : 0;
		const linesPerSec = filesPerSec * linesInEnable;
		console.log(`files/s: ${filesPerSec.toFixed(2)}`);
		console.log(`lines/s: ${Math.floor(linesPerSec)}`);
		console.log(`rss delta mean/min/max (bytes): ${Math.floor(s.meanRssDelta)} / ${s.minRssDelta} / ${s.maxRssDelta}`);
	}

	// Preload the full dataset, then search using separate list
	void trie.insertFromFile(filePath);
	console.log("\nRound 2/3: searchFromFile search.txt over enable1 dataset");
	{
		const linesInSearch = fs.readFileSync(searchFilePath, "utf8").split("\n").length;
		const s = await measureFunctionWithMemory(() => {
			void trie.searchFromFile(searchFilePath);
		}, { iterations: iters, lockGc: withGc, warmupIterations: 1 });
		console.log(formatSample("searchFromFile search.txt", s));
		const filesPerSec = s.meanNs > 0 ? (1_000_000_000 / s.meanNs) : 0;
		const linesPerSec = filesPerSec * linesInSearch;
		console.log(`files/s: ${filesPerSec.toFixed(2)}`);
		console.log(`lines/s: ${Math.floor(linesPerSec)}`);
		console.log(`rss delta mean/min/max (bytes): ${Math.floor(s.meanRssDelta)} / ${s.minRssDelta} / ${s.maxRssDelta}`);
	}

	console.log("\nRound 3/3: removeFromFile remove.txt from enable1 dataset");
	{
		const linesInRemove = fs.readFileSync(removeFilePath, "utf8").split("\n").length;
		const s = await measureFunctionWithMemory(() => {
			void trie.removeFromFile(removeFilePath);
		}, { iterations: iters, lockGc: withGc, warmupIterations: 1 });
		console.log(formatSample("removeFromFile remove.txt", s));
		const filesPerSec = s.meanNs > 0 ? (1_000_000_000 / s.meanNs) : 0;
		const linesPerSec = filesPerSec * linesInRemove;
		console.log(`files/s: ${filesPerSec.toFixed(2)}`);
		console.log(`lines/s: ${Math.floor(linesPerSec)}`);
		console.log(`rss delta mean/min/max (bytes): ${Math.floor(s.meanRssDelta)} / ${s.minRssDelta} / ${s.maxRssDelta}`);
	}
}

main().catch((e) => {
	console.error(e);
	process.exit(1);
});


