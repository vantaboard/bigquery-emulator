# conformance/fixtures/fastpath/

Fast-path fixtures for the DuckDB-native / DuckDB-rewrite routes.

Every YAML in this directory exercises a `ResolvedAST` shape that the
DuckDB fast path is supposed to lower (per
`backend/engine/duckdb/transpiler/SHAPE_TRACKER.md`). The split from
the seed set in `conformance/fixtures/` is two-fold:

* **Per-shape coverage.** Each supported node kind on the
  `duckdb_native` / `duckdb_rewrite` route gets at least one fixture
  here that pins its semantics (row order, NULL handling, alias
  flow). The seed set covers a handful of shapes incidentally; the
  fastpath subdirectory aims for a clean 1:1 map between the
  SHAPE_TRACKER row and at least one fixture.

* **Regression suite for sharp edges.** Fixtures named
  `regression_*.yaml` pin behaviors that historically degraded
  silently when DuckDB's defaults differed from BigQuery's: NaN
  ordering, NULL-equality in joins, integer overflow, nested STRUCT
  literal field order, anonymous-field STRUCT round-tripping.

## How this directory is used

`task conformance:fastpath` runs only this subtree (~2 minutes,
PR-gating). The full conformance run (`task conformance:run`) picks
up everything under `conformance/fixtures/` so these fixtures also
participate in the slower full-suite gate.

## Route-label assertion (planned)

Once `conformance-routing-matrix.plan.md` (plan 16) lands, the
fixture loader / runner will gain a `route_label:` field on
`Expectation` and a hard assertion that fixtures here all resolve to
either `duckdb_native` or `duckdb_rewrite`. Until then we keep the
fixtures as-is and rely on the directory split to keep the fast-path
filter precise. Adding a manual `route_label:` field today would
churn unnecessarily on every fixture when plan 16 wires the real
assertion; the comment block here is the placeholder.

## Adding a fixture

* Put the fixture under a name that matches its node kind family
  (`scan_*.yaml`, `expr_*.yaml`, `regression_*.yaml`).
* Use `match: ordered` plus an explicit `ORDER BY` whenever the
  query reads from a table; DuckDB returns rows in
  implementation-defined order otherwise.
* Use `match: schema_only` for non-deterministic shapes (sampling,
  CURRENT_*).
* Land the fixture in the same commit as any disposition or
  transpiler change it pins.
