---
name: Parity 12 — Pipe operators + specialized families
overview: Land the newest-syntax and specialized-family stragglers - the pipe subpipeline evaluator (|> IF / FORK / TEE / INSERT), WITH GROUP ROWS, deferred computed columns, MATCH_RECOGNIZE, FLATTEN, inline lambdas (ARRAY_TRANSFORM/ARRAY_FILTER), and the HLL_COUNT.* / NET.* semantic bodies that ENGINE_POLICY marks local_impl.
depends_on: [parity-01-unnest-lateral-correlated, parity-04-insert-select-dml, parity-09-ddl-control-op]
est_effort: ~2-3 weeks
isProject: true
todos:
  - id: inline-lambda
    content: "ResolvedInlineLambda + lambda-arg functions (ARRAY_TRANSFORM, ARRAY_FILTER, ...): evaluate lambdas on the semantic executor binding lambda params via FrameStack; most-used item in this plan, land first."
    status: pending
  - id: net-hll
    content: "NET.* (ip/host parsing) and HLL_COUNT.* (INIT/MERGE/EXTRACT with BigQuery-compatible sketch format) bodies on the semantic executor; drop their status=planned markers in functions.yaml."
    status: pending
  - id: subpipeline-evaluator
    content: "Generic subpipeline evaluator for ResolvedPipeIfScan / ResolvedPipeForkScan / ResolvedPipeTeeScan replacing the focused kNotImplemented stubs in the semantic executor."
    status: pending
  - id: pipe-dml-ddl
    content: "ResolvedPipeInsertScan via the DML executor (needs parity-04); real handlers for pipe EXPORT DATA / CREATE TABLE delegating to parity-09's writer + CTAS machinery (replacing the kUnimplemented stubs in backend/engine/control/pipe_*.cc)."
    status: pending
  - id: group-rows
    content: "WITH GROUP ROWS (ResolvedGroupRowsScan): per-group row-set evaluator feeding the GROUP_ROWS() TVF inside aggregate calls."
    status: pending
  - id: deferred-computed-column
    content: "ResolvedDeferredComputedColumn deferred-error-capture semantics (side_effect_column) on the semantic executor."
    status: pending
  - id: match-recognize
    content: "ResolvedMatchRecognizeScan: row-pattern matching on the semantic executor (DEFINE/PATTERN/MEASURES subset BigQuery documents); largest single item - consider its own sub-plan if scope balloons."
    status: pending
  - id: flatten
    content: "ResolvedFlatten / ResolvedFlattenedArg (implicit array flattening in path expressions) on the semantic executor."
    status: pending
  - id: fixtures-trackers
    content: Fixtures per shape; flip SHAPE_TRACKER + node_dispositions.yaml + functions.yaml rows; update ENGINE_POLICY family table rows (NET/HLL move from 'body not yet implemented' to landed).
    status: pending
---

# Parity 12 — Pipe operators + specialized families

## Why last (almost)

These shapes have explicit planned routes and scaffolding (classifier
promotion, focused `kNotImplemented` stubs, control-op pre-dispatch),
but they are the least-encountered surfaces in real workloads: pipe
syntax is new, MATCH_RECOGNIZE / GROUP ROWS are niche, and HLL/NET are
specialized families. Inline lambdas are the exception — `ARRAY_FILTER`
/ `ARRAY_TRANSFORM` see real use, hence first within the plan.

## Current stubs to replace

| Shape | Stub today |
|-------|-----------|
| `ResolvedPipeIfScan` / `ForkScan` / `TeeScan` | classifier promotes; executor surfaces focused `kNotImplemented` until "the subpipeline evaluator lands" |
| `ResolvedPipeExportDataScan` / `PipeCreateTableScan` | control-op pre-dispatch to `pipe_export_data.cc` / `pipe_create_table.cc`, both `kUnimplemented` |
| `ResolvedPipeInsertScan` | `(planned)` semantic_executor |
| `ResolvedGroupRowsScan` | promoted; `kNotImplemented` until "per-group row-set evaluator lands" |
| `ResolvedDeferredComputedColumn` | promoted; UNIMPLEMENTED |
| `ResolvedMatchRecognizeScan` | `(planned)` semantic_executor |
| `ResolvedFlatten` / `FlattenedArg` / `ResolvedInlineLambda` | `(planned)` semantic_executor |
| `NET.*` / `HLL_COUNT.*` | `local_impl` posture, "body not yet implemented" |

## Key files

- [`backend/engine/semantic/`](../../backend/engine/semantic/) — subpipeline evaluator, lambda eval, `StripBarrierScans` neighborhood
- [`backend/engine/control/pipe_export_data.cc`](../../backend/engine/control/), `pipe_create_table.cc`
- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc)
- `backend/engine/duckdb/transpiler/functions.yaml` — NET/HLL rows
- `conformance/fixtures/`

## Steps

1. Inline lambdas + the functions they unlock (highest usage here).
2. NET.* (self-contained string/ip parsing) then HLL_COUNT.* (define a
   stable local sketch format; BigQuery's wire sketches need not be
   byte-compatible — document the divergence posture in ENGINE_POLICY
   if sketches are not interchangeable with real BigQuery).
3. Subpipeline evaluator (shared by IF/FORK/TEE), then the pipe DML/DDL
   delegations to machinery from plans 04/09.
4. GROUP ROWS + deferred computed columns.
5. MATCH_RECOGNIZE last; time-box and split into its own plan if the
   pattern-automaton work exceeds the estimate.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task conformance:routing-matrix
task lint:dispositions
task bazel:shutdown && task bazel:status
```

## Out of scope

- Graph (GQL), proto shapes, MEASURE, sequences, DP aggregates, ML.*, ST_*, KEYS.ENCRYPT — `unsupported` by design per ENGINE_POLICY; no plan moves them without a deliberate policy change
- `ResolvedRelationArgumentScan` — lands with module/TVF relation args only if plan 08 surfaces a concrete need; otherwise tracker note
