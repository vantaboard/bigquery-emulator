---
name: Expand 06 — Privacy-preserving aggregates (stubs)
overview: Differential privacy only matters in production; the local emulator cannot honor the DP guarantee and there is no value in modeling it. The goal here is only to stop these scans from failing a query. Stub ResolvedAnonymizedAggregateScan, ResolvedDifferentialPrivacyAggregateScan, and ResolvedAggregationThresholdAggregateScan so a query that uses WITH ANONYMIZATION / DIFFERENTIAL_PRIVACY / AGGREGATION_THRESHOLD returns a deterministic result (the underlying aggregate, no noise/suppression) instead of UNIMPLEMENTED.
est_effort: ~1 week
isProject: true
todos:
  - id: stub-semantics
    content: "Decide the no-fail behavior: parse + ignore the privacy parameters (epsilon/delta/k_threshold/privacy_unit_column) and evaluate the underlying aggregation as a plain GROUP BY aggregate on the semantic executor, emitting the documented output columns. No noise, no group suppression. Explicitly a placeholder, NOT a privacy guarantee."
    status: pending
  - id: stub-scans
    content: "Route the three scan classes (ResolvedAnonymizedAggregateScan, ResolvedDifferentialPrivacyAggregateScan, ResolvedAggregationThresholdAggregateScan) to the semantic executor's existing aggregate-scan eval with the privacy modifiers stripped, so the query produces rows instead of erroring."
    status: pending
  - id: fixtures-trackers
    content: "Conformance fixtures: a WITH ANONYMIZATION / DIFFERENTIAL_PRIVACY / AGGREGATION_THRESHOLD query returns the plain aggregate without erroring. Flip the three scan rows from unsupported -> local_stub (or semantic_executor-as-stub) in node_dispositions.yaml + SHAPE_TRACKER; update the ENGINE_POLICY DP row to describe the no-fail stub (and that it is NOT differential privacy) + ROADMAP §Privacy-preserving aggregates."
    status: pending
  - id: skip-audit
    content: "Third-party + conformance skip audit (run before declaring done). The stub makes DP/anonymized/threshold queries succeed (plain aggregate), so check for currently-skipped or known_failing cases that now run: sweep the bqutils corpus known_failing/ (conformance/thirdparty-fixtures/bigquery_utils/known_failing/) and the GoogleSQL `.test` corpus for DP-clause fixtures; re-run any third-party subtest that issues these clauses. Only unskip where the test asserts the query *runs* (not a privacy guarantee); note why for the rest. Update third_party/README.md."
    status: completed
---

# Expand 06 — Privacy-preserving aggregates (stubs)

## Why

Differential privacy is a production-only concern: the guarantee depends
on calibrated noise the emulator cannot honor, and a local test harness
gains nothing from modeling it. The product decision (ROADMAP
§Privacy-preserving aggregates) is therefore **stub, do not implement**:
the only goal is that a query using `WITH ANONYMIZATION` /
`DIFFERENTIAL_PRIVACY` / `WITH AGGREGATION_THRESHOLD` **does not fail**.

## The hard part

Being unambiguous that this is not differential privacy. The stub
strips the privacy modifiers and returns the plain underlying
aggregate — no noise, no suppression. The ENGINE_POLICY DP row and any
result-adjacent docs must say so plainly so nobody mistakes the
emulator's output for a privacy-preserving result.

## Key files

- [`backend/engine/semantic/scan_eval_scan_impl.cc`](../../backend/engine/semantic/) — aggregate scan evaluation to reuse
- [`backend/engine/semantic/functions/aggregate_specialized.cc`](../../backend/engine/semantic/functions/aggregate_specialized.cc) — aggregate eval
- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc) — scan dispatch (strip privacy modifiers)
- [`backend/engine/duckdb/transpiler/node_dispositions.yaml`](../../backend/engine/duckdb/transpiler/node_dispositions.yaml) — the three scan rows
- [`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md) — DP row

## Steps

1. Decide the no-fail behavior (plain aggregate, modifiers ignored).
2. Route the three scans to aggregate eval with modifiers stripped.
3. Fixtures (no-error) + flip the rows + doc updates ("not DP").

## Third-party / conformance to revisit

The stub makes these clauses succeed, so **audit for newly-runnable
tests** — but only unskip where the test checks the query *runs*, not a
privacy guarantee. Re-run to prove it; note why for the rest.

- **bqutils + GoogleSQL `.test` corpus** — sweep
  `conformance/thirdparty-fixtures/bigquery_utils/known_failing/` and
  `conformance/googlesql-corpus/` for DP-clause fixtures.
- **client lanes** — re-run any subtest issuing `WITH ANONYMIZATION` /
  `DIFFERENTIAL_PRIVACY` / `AGGREGATION_THRESHOLD`; update
  `third_party/README.md`.

## Out of scope

- Any actual differential-privacy guarantee, calibrated noise, group
  suppression, or privacy-budget accounting.
