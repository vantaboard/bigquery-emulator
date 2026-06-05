---
name: ""
overview: ""
todos: []
isProject: false
---

# Engine Router Foundation

## Goal

Install an explicit route classifier behind `backend/engine/engine.h`
so the in-process implementation is no longer "always the DuckDB
fast path." Split the engine implementation into route-aware
components, but keep the public `Engine` interface and the gRPC
contract unchanged so the gateway, the conformance harness, and the
Storage Read API do not see any change.

## Background

Today `backend/engine/duckdb/duckdb_engine.cc` implements the
`Engine` interface directly: it transpiles every analyzed query to
DuckDB SQL and executes through the DuckDB C++ client. Shapes the
transpiler cannot lower surface `UNIMPLEMENTED` from inside the
transpiler.

After this plan, the same binary still exposes the same `Engine`
interface, but the implementation is a `LocalCoordinatorEngine`
that owns:

- A **route classifier** that, given a resolved-AST root, returns
  the planned `Disposition` for the whole query (a single
  disposition per query; see "Compositional fallback" below).
- One **executor per route** behind a route-typed adapter
  interface: `DuckDbExecutor`, `SemanticExecutor`, `ControlOpExecutor`
  (and an `UnsupportedExecutor` that returns the `UNIMPLEMENTED`
  error directly).

The DuckDB fast path stays as today's primary route; the other
executors land empty in this plan and are filled in by their own
plans (`googlesqlite-07-semantic-core-expr.plan.md`,
`googlesqlite-01-ddl-catalog.plan.md`).

## Dependencies

- `execution-disposition-registry.plan.md` must land first so the
  classifier has a programmatic source of truth.

## Scope

- Add `backend/engine/coordinator/` with:
  - `route_classifier.{h,cc}` — walks the resolved-AST tree, consults
    `node_dispositions_table.inc` and `functions_table.inc`, returns
    one `Disposition` per query plus a structured
    `RouteDecision { disposition; reason; offending_node }`.
  - `executor.h` — the route-typed adapter interface
    (`Execute(const ResolvedAST&, const Catalog&) -> RowSource`).
  - `local_coordinator_engine.{h,cc}` — the new top-level `Engine`
    implementation that owns one instance per executor and dispatches.
- Convert `backend/engine/duckdb/duckdb_engine.cc` from the top-level
  `Engine` impl into a `DuckDbExecutor` adapter behind the new
  `executor.h` interface.
- Add stub `SemanticExecutor`, `ControlOpExecutor`,
  `UnsupportedExecutor` implementations. Stubs return
  `UNIMPLEMENTED` with the disposition-aware error message.
- Wire `binaries/emulator_main/main.cc` to construct a
  `LocalCoordinatorEngine` instead of a `DuckDbEngine` directly.

## Compositional fallback at planning time

The classifier picks one disposition per query. The rule:

- If every node routes `duckdb_native` / `duckdb_rewrite`, route the
  query to `DuckDbExecutor`.
- If any leaf node routes `semantic_executor`, route the **whole
  query** to `SemanticExecutor` (so we never mix strategies
  mid-execution).
- DDL / metadata statements (`control_op` at the root) route to
  `ControlOpExecutor` directly.
- If any leaf node is `unsupported`, route to `UnsupportedExecutor`
  with the offending node's name in the error message.

The fast-path executor can still surface `UNIMPLEMENTED` from inside
its transpiler — that is a fast-path bug, not a classifier bug, and
the classifier rule above is what prevents the runtime-error
re-route hazard the old `FallbackEngine` had.

## Implementation Plan

1. Stand up `backend/engine/coordinator/` with the new interfaces
   and an in-process unit test harness that constructs a coordinator
   with mock executors.
2. Implement `RouteClassifier::Classify(const ResolvedAST&)` using
   the registry from `execution-disposition-registry.plan.md`. Cover
   statement roots, scan recursion, expression recursion, and
   function calls.
3. Convert `DuckDbEngine` into `DuckDbExecutor` (preserve the
   transpiler / DuckDB connection / Arrow result path unchanged).
4. Add the stub `SemanticExecutor`, `ControlOpExecutor`,
   `UnsupportedExecutor` and wire them into the coordinator.
5. Wire `LocalCoordinatorEngine` into `binaries/emulator_main/main.cc`.
6. Update `docs/ENGINE_POLICY.md` "The runtime" table to remove the
   "(planned)" marker on the coordinator / classifier rows.

## Tests

- Unit tests for `RouteClassifier::Classify`:
  - Pure `duckdb_native` query → DuckDB route.
  - Query containing a `semantic_executor` function → semantic
    route with the offending node recorded.
  - DDL root → control-op route.
  - `unsupported` function in a SELECT → unsupported route with the
    function name in the reason.
- Integration test that a representative SELECT round-trips through
  the coordinator and produces identical wire output to the previous
  direct `DuckDbEngine` path.
- Regression test: every existing engine-level integration test
  under `gateway/e2e/` continues to pass against the coordinator.

## Done Criteria

- `LocalCoordinatorEngine` is the only `Engine` implementation
  constructed by `emulator_main`.
- `DuckDbEngine` (top-level engine) is deleted; `DuckDbExecutor`
  (executor adapter) takes its place.
- `RouteClassifier::Classify` returns one of the six dispositions
  for every supported query shape, with the reason recorded.
- All existing conformance fixtures and `gateway/e2e/` tests pass
  unchanged.
- `docs/ENGINE_POLICY.md` "Routes" table no longer marks the
  classifier as "(planned)".
