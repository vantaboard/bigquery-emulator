---
name: ""
overview: ""
todos: []
isProject: false
---

# Execution Disposition Registry

## Goal

Replace the two parallel, drift-prone disposition vocabularies with a
single machine-readable registry that the route classifier, the
shape tracker, the conformance harness, and the function dispatch
table all consult.

Current state:

- `backend/engine/duckdb/transpiler/SHAPE_TRACKER.md` uses the
  six-route vocabulary in the policy-rewrite commit, but the rows are
  hand-maintained and not consumed by code.
- `backend/engine/duckdb/transpiler/functions.yaml` still uses
  `kMap | kFallback | kSkiplist`, generated into
  `functions_table.inc` and consumed at runtime by `functions.cc`.
- The route classifier (after `engine-router-foundation.plan.md`
  lands) needs a programmatic source of truth for "what route does
  this node kind / function take?"

This plan unifies them.

## Scope

- One canonical disposition enum
  (`Disposition::{kDuckdbNative, kDuckdbRewrite, kDuckdbUdf,
  kSemanticExecutor, kControlOp, kUnsupported}`).
- One node-kind disposition table
  (`backend/engine/duckdb/transpiler/node_dispositions.yaml`)
  generated into `node_dispositions_table.inc`.
- The existing `functions.yaml` rewritten to use the same
  vocabulary; `kMap` becomes `duckdb_native` or `duckdb_rewrite`,
  `kFallback` rows get a real planned route, `kSkiplist` becomes
  `unsupported`.
- A `tools/check_disposition_parity.go` (or equivalent) gate that
  fails CI if `SHAPE_TRACKER.md` and `node_dispositions.yaml` drift.

Out of scope (other plans):

- Implementing the route classifier itself
  (`engine-router-foundation.plan.md`).
- Implementing any of the new routes
  (`local-exec-03-operator-disposition.plan.md`,
  `local-exec-07-semantic-core-expr.plan.md`, `local-exec-01-ddl-catalog.plan.md`).

## Implementation Plan

1. Add `backend/engine/disposition.h` with the canonical
   `Disposition` enum and a `DispositionToString` helper.
2. Author
   `backend/engine/duckdb/transpiler/node_dispositions.yaml`
   covering every `ResolvedAST` node kind that appears in
   `SHAPE_TRACKER.md`. Each row carries
   `{node, disposition, plan, notes}`; `(planned)` rows record the
   planned route plus the plan file name.
3. Add a Bazel `genrule` `:node_dispositions_table_inc` that emits
   `node_dispositions_table.inc` (an `absl::flat_hash_map<std::string,
   Disposition>` initializer plus a parallel
   `plan_for_disposition` table).
4. Migrate `functions.yaml` to the same vocabulary: every `kMap` row
   becomes `duckdb_native` (or `duckdb_rewrite` for the small set of
   rewrite-not-direct-call entries); every `kFallback` row picks a
   real planned route (`duckdb_udf` or `semantic_executor`); every
   `kSkiplist` row becomes `unsupported`. Regenerate
   `functions_table.inc`.
5. Update `functions.cc` to expose `LookupFunction` returning the
   new `Disposition` instead of the old enum. Keep the call sites
   working until `engine-router-foundation.plan.md` rewires them.
6. Add `tools/check_disposition_parity` (Go program, run via
   `task lint:dispositions`) that parses `SHAPE_TRACKER.md` and
   `node_dispositions.yaml` and fails when their rows disagree.
   Wire it into the CI lint workflow.
7. Update `SHAPE_TRACKER.md` `## Coverage summary` to point
   readers at the YAML as the authoritative source; the markdown
   table becomes a human-readable mirror.

## Tests

- Unit test for `Disposition` enum / `DispositionToString`.
- Unit test for the generated `node_dispositions_table.inc`: every
  enum value is reachable; every `kUnsupported` row has the same
  plan file name (`local-exec-15-specialized-stubs.plan.md`).
- Unit test for `LookupFunction` returning the new enum.
- Lint test for the parity checker: a deliberately drifted row in
  one source causes the checker to fail.

## Done Criteria

- `node_dispositions.yaml` covers every row currently in
  `SHAPE_TRACKER.md`.
- `functions.yaml` uses the six-route vocabulary; the old
  `kMap` / `kFallback` / `kSkiplist` strings appear nowhere in the
  generated `.inc`.
- `LookupFunction` and the new `LookupNodeDisposition` both return
  the same `Disposition` enum.
- `task lint:dispositions` is green on `main` and is wired into CI.
- `SHAPE_TRACKER.md` "Coverage summary" names the YAML as
  authoritative.
