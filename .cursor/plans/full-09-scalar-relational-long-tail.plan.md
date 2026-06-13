---
name: Full 09 — Scalar & relational long tail
overview: Close the remaining individually-small but collectively-annoying gaps - GENERATE_UUID, extended/type-modifier casts beyond the landed subset, weighted/stratified TABLESAMPLE, and DATE/TIMESTAMP RANGE window frames with non-INTERVAL offsets - each of which still surfaces UNIMPLEMENTED and breaks otherwise-supported queries.
est_effort: ~1-2 weeks
isProject: true
todos:
  - id: generate-uuid
    content: "GENERATE_UUID: promote off `unsupported`. A real local RFC-4122 v4 generator (random per call) is correct BigQuery behavior - the old `stub would lie about uniqueness` rationale only applies to a fixed stub, not a real generator. Implement on the semantic executor, mark non-deterministic so it isn't constant-folded, and pin a fixture asserting format + per-row uniqueness."
    status: pending
  - id: extended-casts
    content: "Extended-cast shapes still surfacing UNIMPLEMENTED (extended_cast() beyond the FORMAT / AT TIME ZONE / type-modifier subset already landed in parity-07): enumerate the remaining ResolvedCast::extended_cast() forms the analyzer produces in PRODUCT_EXTERNAL and land them on eval_expr_cast.cc; keep genuinely-unsupported targets (proto/enum/range/graph/measure) on `unsupported`."
    status: pending
  - id: tablesample-weighted
    content: "Weighted / stratified TABLESAMPLE: ResolvedSampleScan with a weight column or stratified partition list currently surfaces UNIMPLEMENTED. Implement on the semantic executor (DuckDB has no matching SAMPLE form) where BigQuery's weighting semantics are well-defined; document any sampling-distribution caveat."
    status: pending
  - id: range-window-frames
    content: "DATE/TIMESTAMP RANGE window frames with non-INTERVAL offsets: ResolvedAnalyticScan RANGE BETWEEN over DATE/TIMESTAMP ORDER BY keys with numeric (non-INTERVAL) offsets. Land on the semantic executor (the numeric-key RANGE subset already lowers duckdb_native via parity-11)."
    status: pending
  - id: generate-array-edges
    content: "Audit generate_date_array / generate_timestamp_array / range* (already semantic_executor) for any remaining UNIMPLEMENTED edge (step direction, INTERVAL step, empty-range) and pin fixtures; these are listed semantic_executor but verify they're actually landed vs. stubbed."
    status: pending
  - id: fixtures-trackers
    content: "Fixtures per shape under conformance/fixtures/{scalar,sample,window}/; flip generate_uuid in functions.yaml off `unsupported`; update SHAPE_TRACKER ResolvedCast / ResolvedSampleScan / ResolvedAnalyticScan notes (drop the 'still surface UNIMPLEMENTED' caveats that no longer hold); update ROADMAP + ENGINE_POLICY cast/sample sections."
    status: pending
---

# Full 09 — Scalar & relational long tail

## Why

These are the "my query was 99% supported and then one function/clause
killed it" gaps. Each is small, but they're the difference between
"works" and `UNIMPLEMENTED` for real queries. The trackers explicitly
flag them as still-deferred:

- `generate_uuid: unsupported` (functions.yaml) — but a real generator
  is correct, the `unsupported` rationale is stale (it assumed a fixed
  stub).
- SHAPE_TRACKER `ResolvedCast` note: "`extended_cast()` shapes still
  surface `UNIMPLEMENTED`".
- SHAPE_TRACKER `ResolvedSampleScan` note: "Weight columns, stratified
  partition lists ... still surface `UNIMPLEMENTED`".
- SHAPE_TRACKER `ResolvedAnalyticScan` note: "`RANGE` over DATE/TIMESTAMP
  ORDER BY with non-INTERVAL offsets still surface `UNIMPLEMENTED`".

## Key files

- [`backend/engine/semantic/eval_expr_cast.cc`](../../backend/engine/semantic/eval_expr_cast.cc) — extended casts
- [`backend/engine/semantic/functions/`](../../backend/engine/semantic/functions/) — GENERATE_UUID, generate_*_array edges
- [`backend/engine/semantic/scan_eval_scan_impl.cc`](../../backend/engine/semantic/scan_eval_scan_impl.cc) — sample / window-frame evaluation
- [`backend/engine/duckdb/transpiler/functions.yaml`](../../backend/engine/duckdb/transpiler/functions.yaml) + `SHAPE_TRACKER.md`

## Steps

1. GENERATE_UUID (smallest, highest "huh, that doesn't work?" factor).
2. Extended casts — enumerate what the analyzer actually emits, land
   the in-scope ones.
3. Weighted/stratified TABLESAMPLE.
4. DATE/TIMESTAMP RANGE frames.
5. Verify generate_*_array / range* are truly landed; pin edges.
6. Fixtures + flip tracker caveats + doc updates.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task bazel:shutdown && task bazel:status
```

## Out of scope

- Cast targets that are genuinely unsupported families (proto / enum /
  range-of-proto / graph / measure / tokenlist) — stay `unsupported`.
- Sampling reproducibility guarantees beyond `REPEATABLE` (already
  landed in parity-11).
