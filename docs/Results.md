Results
=======

Environment snapshot (example)
------------------------------
- Node: v22.x
- Platform: linux x64
- CPU: Intel(R) Core(TM) i5-4210U (4 threads)
- expose-gc: true

Methodology
-----------
- Each iteration processes a whole file.
- Metrics: mean time, files/s (same as ops/s), lines/s, and per-iteration RSS delta (mean/min/max).
- GC is optionally triggered before iterations to stabilize measurements.
- Config toggles:
  - Normalization can be disabled (`NO_NORMALIZATION=1`).
  - Whitespace can be allowed inside tokens (`ALLOW_WHITESPACE=1`).

Sample results
--------------------------
Insert from file (full dataset)
- mean ≈ 130–135 ms per file
- files/s ≈ 7–8
- lines/s ≈ 1.2–1.3 M
- RSS delta (per iter): ~1.3 MB mean/min/max on this machine

Search from file (search.txt)
- mean ≈ 0.1 ms per file
- files/s ≈ 9,000+
- lines/s determined by input lines
- RSS delta: near zero

Remove from file (remove.txt)
- mean ≈ 0.1 ms per file
- files/s ≈ 9,000+
- lines/s determined by input lines
- RSS delta: near zero

Notes
-----
- Numbers vary by hardware, dataset size, and environment.
- These are indicative only; not performance claims.
 
Trade‑offs observed in full runs
--------------------------------
- Ingestion dominates cost; search/remove are effectively instant (≈0.09–0.18 ms per batch line file) with near‑zero RSS delta.
- Allowing whitespace inside tokens notably speeds up ingestion on term‑like datasets but increases memory usage substantially:
  - On a 101.8 MB terms file, whitespace allowed cut ingest time from ≈6.75–7.04 s to ≈4.75–4.78 s, while increasing RSS delta from ≈223 MB (≈2.19× file) to ≈856 MB (≈8.40× file).
- Disabling normalization yields modest ingest speedups when whitespace is disabled, with similar memory use to the default.
- Token‑dense word datasets can have higher memory ratios (e.g., 28.8 MB words file → ≈395 MB RSS delta, ≈13.7×), reflecting less prefix sharing and more nodes.

Practical guidance
------------------
- If memory is constrained: keep `allowWhitespaceInTokens = false`. You trade ~40–50% slower ingestion for ~3–4× lower memory on term datasets; queries remain just as fast.
- If ingestion speed matters more and RAM is ample: set `allowWhitespaceInTokens = true` to reduce ingest time at the cost of higher memory.
- Normalization: choose based on correctness (case/diacritics). Performance impact is modest compared to whitespace.



Benchmark summary (from docs/results/terms_all.txt)
---------------------------------------------------
These are indicative results captured on an i5-4210U (4 threads), Node v22.17.0, linux x64, with `--expose-gc`, processing each file fully per iteration.

- Terms dataset (101.8 MB):
  - Default (no spaces, normalization on): insert ≈ 7.04 s; RSS delta ≈ 223 MB (≈ 2.19× file)
  - Allow whitespace (normalization on): insert ≈ 4.78 s; RSS delta ≈ 856 MB (≈ 8.40× file)
  - No normalization (no spaces): insert ≈ 6.75 s; RSS delta ≈ 224 MB (≈ 2.20× file)
  - Allow whitespace + no normalization: insert ≈ 4.75 s; RSS delta ≈ 855 MB (≈ 8.40× file)
  - Search/remove across all above: ≈ 0.09–0.18 ms; near-zero RSS delta

- Enable1 dataset (1.74 MB):
  - Insert ≈ 115 ms; RSS delta ≈ 16.4 MB (≈ 9.4× file)
  - Search/remove ≈ 0.095–0.097 ms; near-zero RSS delta

- Words dataset (28.8 MB):
  - Insert ≈ 2.82 s; RSS delta ≈ 395 MB (≈ 13.7× file)
  - Search/remove ≈ 0.10–0.11 ms; near-zero RSS delta

Interpretation
--------------
- Search/remove are effectively free at this scale; capacity planning is driven by ingest time and steady-state memory.
- Allowing whitespace significantly increases memory usage while improving ingest speed on term-like datasets.
- Disabling normalization has a modest effect on ingest time; choose based on correctness needs more than performance.

Bottom line
-----------
- Keep `allowWhitespaceInTokens = false` unless you need whitespace-sensitive tokens or maximum ingest throughput and have ample RAM.
- The default configuration is a balanced choice for most users.

