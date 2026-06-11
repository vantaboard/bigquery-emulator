---
name: Parity 07 — Cast extensions, COLLATE, value tables
overview: Close the deferred expression/projection edges - CAST with FORMAT / AT TIME ZONE / extended casts and type modifiers, COLLATE in ORDER BY, SELECT AS VALUE (is_value_table), and set-operation CORRESPONDING / CORRESPONDING_BY.
est_effort: ~1 week
isProject: true
todos:
  - id: cast-format
    content: "CAST(... AS STRING FORMAT '...') and PARSE-side equivalents: semantic-executor implementation of BigQuery format elements (shared with parity-02's FORMAT_DATE work if it landed first)."
    status: completed
  - id: cast-timezone
    content: "CAST with AT TIME ZONE and extended casts (type modifiers, collation-bearing casts): route to semantic executor per the SHAPE_TRACKER ResolvedCast row; keep the IsCastTargetSupported whitelist for the native path."
    status: completed
  - id: collate
    content: "COLLATE in ORDER BY (ResolvedOrderByItem) and OrderByScan collation: implement und:ci case-insensitive collation on the semantic sort path (or DuckDB collation if exact); pin NULL ordering interaction."
    status: completed
  - id: select-as-value
    content: "SELECT AS VALUE / is_value_table() on ResolvedQueryStmt: emit the single-column value-table projection (struct unwrap) so the gateway serializes the value directly; covers SELECT AS STRUCT subquery interplay already landed."
    status: completed
  - id: setop-corresponding
    content: "ResolvedSetOperationScan column_match_mode=CORRESPONDING / CORRESPONDING_BY: duckdb_rewrite that projects each item's columns into the matched-by-name order before the set operator."
    status: completed
  - id: fixtures-trackers
    content: Fixtures under conformance/fixtures/scalar/ + setops/; flip SHAPE_TRACKER rows (ResolvedCast, ResolvedOrderByItem, ResolvedQueryStmt value-table note, ResolvedSetOperationScan) and node_dispositions.yaml.
    status: completed
  - id: cast-extended-followup
    content: "ResolvedCast extended_cast() / type_modifiers() shapes still surface UNIMPLEMENTED in eval_expr_cast.cc (classifier promotes; evaluator explicit gap)."
    status: pending
    note: "Classifier promotes via VisitResolvedCast; eval_expr_cast.cc returns explicit UNIMPLEMENTED. No conformance fixture yet — STRING(n)/NUMERIC(p,s) deferred to a follow-up plan."
---

# Parity 07 — Cast extensions, COLLATE, value tables

## Why

These are the remaining deferred edges on shapes that are otherwise
landed, so each is a contained increment with existing scaffolding:

- SHAPE_TRACKER `ResolvedCast`: *"Format / time-zone / extended-cast /
  type-modifier / collation cases reroute to semantic_executor
  (planned)"*.
- `ResolvedOrderByItem` / `ResolvedOrderByScan`: *"COLLATE reroutes to
  semantic_executor (planned)"*.
- `ResolvedQueryStmt`: *"`is_value_table()` queries (SELECT AS VALUE)
  still surface UNIMPLEMENTED"* — value tables show up in client-library
  tests and UDF bodies.
- `ResolvedSetOperationScan`: *"column_match_mode=CORRESPONDING /
  CORRESPONDING_BY reroutes to duckdb_rewrite (planned)"*.

## Key files

- [`backend/engine/duckdb/transpiler/`](../../backend/engine/duckdb/transpiler/) — `EmitQueryStmt` (value tables), set-op emit, cast emit + `IsCastTargetSupported`
- [`backend/engine/semantic/`](../../backend/engine/semantic/) `eval_expr.cc` — cast evaluation branches
- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc)
- `conformance/fixtures/scalar/`, `conformance/fixtures/setops/`

## Steps

1. `SELECT AS VALUE` first (most visible): the value-table projection
   is a transpiler/output-schema change — the outermost output column
   is the value itself; verify gateway wire serialization for STRUCT
   value tables matches BigQuery (positional `f`/`v`).
2. CORRESPONDING rewrite: pure transpiler work; per-item projection
   onto the name-intersection (CORRESPONDING) or the BY list, then the
   existing set-op join; analyzer already validates name sets.
3. Cast FORMAT / AT TIME ZONE: implement on the semantic executor;
   share the format-element engine with parity-02 if available.
4. COLLATE: probe DuckDB's ICU collation for exactness on `und:ci`;
   if exact, lower natively; else sort on the semantic path.
5. Fixtures + tracker flips per edge.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task bazel:shutdown && task bazel:status
```

## Out of scope

- Casts to GEOGRAPHY / proto / enum / range / graph / measure — stay `unsupported` per ENGINE_POLICY
- General collation support in joins/group-by (only ORDER BY scoped here; note further gaps in the tracker if the analyzer surfaces them)
