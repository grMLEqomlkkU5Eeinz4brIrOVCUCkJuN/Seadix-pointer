import os from "os";

export type MeasureSample = {
	iterations: number;
	meanNs: number;
	stdDevNs: number;
	cov: number;
	minNs: number;
	maxNs: number;
};

export type MeasureSampleWithMem = MeasureSample & {
    meanRssDelta: number;
    minRssDelta: number;
    maxRssDelta: number;
};

export type MeasureOptions = {
	iterations: number;
	warmupIterations?: number;
	lockGc?: boolean;
	repeats?: number;
};

export function nowNs(): bigint {
	return process.hrtime.bigint();
}

export function nsToMs(ns: number): number {
	return ns / 1_000_000;
}

function mean(values: number[]): number {
	if (values.length === 0) return 0;
	return values.reduce((a, b) => a + b, 0) / values.length;
}

function stddev(values: number[], meanValue: number): number {
	if (values.length < 2) return 0;
	const variance =
		values.reduce((acc, v) => acc + (v - meanValue) * (v - meanValue), 0) /
		(values.length - 1);
	return Math.sqrt(variance);
}

export function ensureGcExposed(): void {
	if (typeof global.gc !== "function") {
		throw new Error(
			"GC not exposed. Run node with --expose-gc (e.g., npm script uses node --expose-gc).",
		);
	}
}

export async function measureFunction(
	fn: () => void | Promise<void>,
	opts: MeasureOptions,
): Promise<MeasureSample> {
	const iterations = Math.max(1, opts.iterations);
	const warmupIterations = Math.max(0, opts.warmupIterations ?? Math.min(50, Math.floor(iterations * 0.1)));
	const lockGc = opts.lockGc === true;

	// Warmup to trigger JIT and caches
	for (let i = 0; i < warmupIterations; i++) {
		 
		await fn();
	}

	const samples: number[] = [];
	let minNs = Number.POSITIVE_INFINITY;
	let maxNs = 0;

	for (let i = 0; i < iterations; i++) {
		if (lockGc) {
			// Encourage a stable GC state before the sample
			if (typeof global.gc === "function") {
				global.gc();
			}
		}
		const start = nowNs();
		 
		await fn();
		const end = nowNs();
		const delta = Number(end - start);
		samples.push(delta);
		if (delta < minNs) minNs = delta;
		if (delta > maxNs) maxNs = delta;
	}

	const meanNs = mean(samples);
	const stdDevNs = stddev(samples, meanNs);
	const cov = meanNs === 0 ? 0 : stdDevNs / meanNs;

	return { iterations, meanNs, stdDevNs, cov, minNs, maxNs };
}

export async function measureFunctionWithMemory(
	fn: () => void | Promise<void>,
	opts: MeasureOptions,
): Promise<MeasureSampleWithMem> {
	const iterations = Math.max(1, opts.iterations);
	const warmupIterations = Math.max(0, opts.warmupIterations ?? Math.min(50, Math.floor(iterations * 0.1)));
	const lockGc = opts.lockGc === true;

	for (let i = 0; i < warmupIterations; i++) {
		 
		await fn();
	}

	const timeSamples: number[] = [];
	const rssDeltas: number[] = [];
	let minNs = Number.POSITIVE_INFINITY;
	let maxNs = 0;
	let minRss = Number.POSITIVE_INFINITY;
	let maxRss = 0;

	for (let i = 0; i < iterations; i++) {
		if (lockGc && typeof global.gc === "function") {
			global.gc();
		}
		const rssBefore = process.memoryUsage().rss;
		const start = nowNs();
		 
		await fn();
		const end = nowNs();
		const rssAfter = process.memoryUsage().rss;
		const deltaNs = Number(end - start);
		const deltaRss = Math.max(0, rssAfter - rssBefore);
		timeSamples.push(deltaNs);
		rssDeltas.push(deltaRss);
		if (deltaNs < minNs) minNs = deltaNs;
		if (deltaNs > maxNs) maxNs = deltaNs;
		if (deltaRss < minRss) minRss = deltaRss;
		if (deltaRss > maxRss) maxRss = deltaRss;
	}

	const meanNs = mean(timeSamples);
	const stdDevNs = stddev(timeSamples, meanNs);
	const cov = meanNs === 0 ? 0 : stdDevNs / meanNs;
	const meanRssDelta = mean(rssDeltas);

	return { iterations, meanNs, stdDevNs, cov, minNs, maxNs, meanRssDelta, minRssDelta: minRss, maxRssDelta: maxRss };
}

export function formatSample(title: string, s: MeasureSample): string {
	const opsPerSec = s.meanNs > 0 ? (1_000_000_000 / s.meanNs) : 0;
	return [
		`\n=== ${title} ===`,
		`iterations: ${s.iterations}`,
		`mean: ${nsToMs(s.meanNs).toFixed(3)} ms`,
		`ops/s: ${opsPerSec.toFixed(2)}`,
		`stddev: ${nsToMs(s.stdDevNs).toFixed(3)} ms`,
		`cov: ${(s.cov * 100).toFixed(2)}%`,
		`min: ${nsToMs(s.minNs).toFixed(3)} ms`,
		`max: ${nsToMs(s.maxNs).toFixed(3)} ms`,
	].join("\n");
}

export function printRuntimeContext(): void {
	// Provide evidence for fairness: environment snapshot
	// Users may also pin CPU governor externally for stricter control
	// We cannot change system settings from here, but we disclose context
	console.log("Environment:");
	console.log(`node: ${process.version}`);
	console.log(`platform: ${process.platform} ${process.arch}`);
	console.log(`cpu (threads): ${os.cpus()?.[0]?.model ?? "unknown"} x${os.cpus()?.length ?? 0}`);
	console.log(`memory: total ${(os.totalmem() / (1024 ** 3)).toFixed(2)} GiB`);
	console.log(`expose-gc: ${typeof global.gc === "function"}`);
}

export async function runAndReport(title: string, fn: () => void | Promise<void>, opts: MeasureOptions): Promise<void> {
	const s = await measureFunction(fn, opts);
	console.log(formatSample(title, s));
}

export function assertOk(condition: unknown, message: string): void {
	if (!condition) throw new Error(message);
}


