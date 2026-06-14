---
name: Expand 06 — Privacy-preserving aggregates
overview: Promote the differential-privacy / anonymized / aggregation-threshold scan family off unsupported. Implement ResolvedAnonymizedAggregateScan, ResolvedDifferentialPrivacyAggregateScan, and ResolvedAggregationThresholdAggregateScan on the semantic executor as a local, deterministic implementation suitable for tests - explicitly NOT a production DP guarantee.
est_effort: ~2-3 weeks
isProject: true
todos:
  - id: semantics-decision
    content: "Decide the local semantics. BigQuery's DP guarantee depends on calibrated noise the emulator cannot honor. Two options: (a) deterministic mode (seeded/zero noise) so conformance is reproducible, with the privacy parameters parsed + validated but the noise made deterministic; (b) seeded-PRNG noise for shape realism. Pick deterministic-by-default and document loudly that this is NOT a privacy guarantee."
    status: pending
  - id: anon-aggregate
    content: "ResolvedAnonymizedAggregateScan: WITH ANONYMIZATION aggregation — parse the privacy params (epsilon/delta/k_threshold), evaluate the underlying aggregates on the semantic executor, apply the aggregation threshold (drop groups below the contribution threshold), and apply deterministic noise per the chosen mode."
    status: pending
  - id: dp-aggregate
    content: "ResolvedDifferentialPrivacyAggregateScan: the newer DIFFERENTIAL_PRIVACY clause form (privacy_unit_column, epsilon/delta budget). Share the aggregation-threshold + noise machinery with the anonymized path."
    status: pending
  - id: threshold-aggregate
    content: "ResolvedAggregationThresholdAggregateScan: WITH AGGREGATION_THRESHOLD — group suppression below a count threshold without the full DP noise apparatus. Simplest of the three; good first landing."
    status: pending
  - id: fixtures-trackers
    content: "Conformance fixtures: aggregation-threshold suppression, anonymized aggregate group-drop + deterministic output, DP-clause variant. Flip the three scan rows off unsupported in node_dispositions.yaml + SHAPE_TRACKER; update the ENGINE_POLICY DP row (note deterministic, not a privacy guarantee) + ROADMAP §Privacy-preserving aggregates."
    status: pending
---

# Expand 06 — Privacy-preserving aggregates

## Why

[ROADMAP.md §Privacy-preserving aggregates](../../ROADMAP.md) tracks the
three scans as ⏳ planned. ENGINE_POLICY marks them `unsupported` because
"the DP guarantee depends on noise calibration the emulator cannot
honor." For an emulator the **useful** behavior is deterministic + test-
reproducible result shape (group suppression below thresholds, correct
output columns) — not an actual privacy guarantee.

## The hard part

Honesty about the contract. We must implement the *shape* (parse privacy
params, suppress small groups, emit the documented columns) while making
absolutely clear — in the envelope-adjacent docs and the ENGINE_POLICY
row — that the local implementation does **not** provide differential
privacy. Deterministic-by-default noise keeps conformance stable; an
opt-in seeded-PRNG mode can come later.

## Key files

- [`backend/engine/semantic/functions/aggregate_specialized.cc`](../../backend/engine/semantic/functions/aggregate_specialized.cc) — specialized aggregate home
- [`backend/engine/semantic/scan_eval_scan_impl.cc`](../../backend/engine/semantic/) — aggregate scan evaluation
- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc) — scan dispatch
- [`backend/engine/duckdb/transpiler/node_dispositions.yaml`](../../backend/engine/duckdb/transpiler/node_dispositions.yaml) — the three scan rows
- [`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md) — DP row

## Steps

1. Decide local semantics (deterministic-by-default) + document.
2. `ResolvedAggregationThresholdAggregateScan` (simplest; land first).
3. `ResolvedAnonymizedAggregateScan` (threshold + deterministic noise).
4. `ResolvedDifferentialPrivacyAggregateScan` (DP-clause variant).
5. Fixtures + tracker/posture flips + the "not a privacy guarantee" note.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task bazel:shutdown && task bazel:status
```

## Out of scope

- Any actual differential-privacy guarantee / calibrated noise.
- Per-user contribution bounding beyond what the threshold needs.
- Privacy budget accounting across queries.
