---
name: Full 08 — MATCH_RECOGNIZE & remaining planned AST shapes
overview: Clear the last status=planned rows in node_dispositions.yaml - ResolvedMatchRecognizeScan, ResolvedUpdateConstructor, ResolvedRelationArgumentScan, ResolvedCatalogColumnRef - plus MERGE THEN RETURN, landing each on its planned route with conformance coverage so the tracker has zero (planned) rows for in-scope shapes.
est_effort: ~2-3 weeks
isProject: true
todos:
  - id: match-recognize
    content: "ResolvedMatchRecognizeScan (semantic_executor, status=planned): implement row-pattern matching on the semantic executor - PARTITION BY / ORDER BY, DEFINE predicates, PATTERN regex over row variables, MEASURES projection, and the ONE ROW PER MATCH / ALL ROWS PER MATCH output modes. This is the largest item; scope an initial subset (single-pattern, ONE ROW PER MATCH) first and pin the rest as follow-on UNIMPLEMENTED."
    status: pending
  - id: update-constructor
    content: "ResolvedUpdateConstructor (semantic_executor, status=planned): the DML expression form used by nested UPDATE constructors; wire into backend/engine/semantic/dml/ and pin with a fixture."
    status: pending
  - id: relation-argument-scan
    content: "ResolvedRelationArgumentScan (semantic_executor, status=planned): table/relation-typed arguments inside TVF bodies; evaluate against the EvalContext relation-argument frame the TVF caller pushes (mirror the ResolvedArgumentRef scalar path). Pin with a TVF-with-table-arg fixture."
    status: pending
  - id: catalog-column-ref
    content: "ResolvedCatalogColumnRef (control_op, marked (planned) in SHAPE_TRACKER): find/author a DDL option-list shape that produces it (e.g. ALTER TABLE ... SET OPTIONS referencing a column) and either land EmitCatalogColumnRef or, if no PRODUCT_EXTERNAL parse surface produces it, drop the (planned) marker with a note that no consumer exists."
    status: pending
  - id: merge-then-return
    content: "MERGE THEN RETURN: deferred in parity-06 pending pinned googlesql exposing ResolvedMergeStmt::returning(). Re-check the prebuilt googlesql artifact; if returning() is now exposed, wire MERGE RETURNING through dml_returning.cc like INSERT/UPDATE/DELETE; otherwise document the upstream blocker explicitly."
    status: pending
  - id: fixtures-trackers
    content: "Add conformance fixtures per landed shape; drop status=planned / (planned) from node_dispositions.yaml + SHAPE_TRACKER rows; update ROADMAP DML + relational bullets; keep task lint:dispositions green."
    status: pending
---

# Full 08 — MATCH_RECOGNIZE & remaining planned AST shapes

## Why

These are the last `status=planned` rows in the in-scope AST surface
(graph / proto / DP / measure families stay deliberately `unsupported`):

```112:115:backend/engine/duckdb/transpiler/node_dispositions.yaml
ResolvedMatchRecognizeScan: semantic_executor status=planned
ResolvedGroupRowsScan: semantic_executor
ResolvedTVFScan: semantic_executor
ResolvedRelationArgumentScan: semantic_executor status=planned
```

```173:173:backend/engine/duckdb/transpiler/node_dispositions.yaml
ResolvedUpdateConstructor: semantic_executor status=planned
```

Plus `ResolvedCatalogColumnRef` (marked `(planned)` in SHAPE_TRACKER) and
MERGE `THEN RETURN` (deferred in parity-06 on an upstream googlesql
dependency). Clearing these means the tracker has no half-landed rows
for shapes the emulator intends to support.

## Key files

- [`backend/engine/semantic/`](../../backend/engine/semantic/) — MATCH_RECOGNIZE evaluator, relation-argument frame
- [`backend/engine/semantic/dml/dml_returning.cc`](../../backend/engine/semantic/dml/), `dml_merge.cc` — MERGE RETURNING
- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc) — promotion for the new shapes
- [`backend/engine/duckdb/transpiler/node_dispositions.yaml`](../../backend/engine/duckdb/transpiler/node_dispositions.yaml) + `SHAPE_TRACKER.md`
- `docs/bigquery/docs/reference/standard-sql/` — MATCH_RECOGNIZE / pattern syntax

## Steps

1. Land the small shapes first (UpdateConstructor, RelationArgumentScan,
   CatalogColumnRef, MERGE RETURNING if unblocked) — fast tracker wins.
2. Then MATCH_RECOGNIZE: scope `ONE ROW PER MATCH` single-pattern first;
   leave the rest `UNIMPLEMENTED` with a written note rather than a
   half-correct matcher.
3. Fixtures + drop the `status=planned` / `(planned)` markers in the
   same commit as each implementation.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions     # confirm zero planned rows for landed shapes
task bazel:shutdown && task bazel:status
```

## Out of scope

- Graph (`ResolvedGraph*Scan`), proto (`ResolvedMakeProto`, ...),
  differential-privacy aggregate scans, `ResolvedSequence`,
  `ResolvedGetRowField` (MEASURE) — all remain `unsupported` by design.
- Full MATCH_RECOGNIZE `ALL ROWS PER MATCH` + AFTER MATCH SKIP variants
  if the initial subset proves large; pin as follow-on.
