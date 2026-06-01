---
name: ""
overview: ""
todos: []
isProject: false
---

# Conformance Routing Matrix

## Goal

Make the route the coordinator used to execute a query observable in
test output. Fixtures assert not just expected rows / errors, but
which route ran — so a passing fixture cannot hide accidental drift
between strategies (a query that used to run on the DuckDB fast path
silently re-routes to the semantic executor and slows down, or vice
versa with semantic-only behavior silently lost on a fast-path
demotion).

## Background

The conformance harness (`conformance/cmd/runner`,
`conformance/fixtures/*.yaml`) today diffs `expected.rows` against
the gateway's wire response. There is no per-query observation of
which engine route served the result.

After `engine-router-foundation.plan.md` lands, every query carries
a `RouteDecision`. This plan exposes that decision on the wire (only
to the conformance harness, not to BigQuery REST clients) and
teaches fixtures to assert it.

## Dependencies

- `engine-router-foundation.plan.md` (so there is a `RouteDecision`
  to expose).
- `execution-disposition-registry.plan.md` (so the route names are
  the same across docs / tracker / harness).

## Scope

- A new debug field on `Job.statistics.query` (`emulatorRoute`)
  that the conformance harness uses to read back the route. The
  field is omitted from REST responses to non-`localhost` callers
  so it stays internal.
- A fixture-schema field (`expected.route: <duckdb_native | ...>`)
  that the runner compares against the response's
  `emulatorRoute`.
- A new fixture flag `expected.route_strict: bool` (default
  `true`) — when `false`, the runner only enforces that the route
  is in a small allowlist (used for shapes that are deliberately
  flexible between, say, `duckdb_native` and `duckdb_rewrite`).
- A `task conformance:routing-matrix` Task that prints a matrix of
  shape × route from every fixture's expected route.
- Conformance fixtures backfilled with `expected.route` for every
  existing fixture.

## Implementation Plan

1. Add `emulatorRoute` to `gateway/bqtypes/` and surface it from
   `LocalCoordinatorEngine` via the gRPC `QueryResultMetadata`.
2. Gate `emulatorRoute` on a loopback-only middleware check in
   `gateway/middleware/` so the field is omitted from REST
   responses to non-loopback callers.
3. Extend the conformance fixture schema in
   `conformance/cmd/runner/`:
   - `expected.route` (single value).
   - `expected.route_strict` (default `true`).
4. Teach the runner to compare the response's `emulatorRoute`
   against the fixture; on mismatch, fail with a diagnostic that
   names both the actual and expected route.
5. Backfill `expected.route` on every existing fixture. Pin
   `duckdb_native` for fast-path fixtures; pin
   `semantic_executor` for the few fixtures that already exercise
   semantic-only behavior.
6. Add `task conformance:routing-matrix` that walks every fixture
   and emits a Markdown table of shape × route. Wire it into CI
   as a non-blocking artifact for visibility.

## Tests

- Runner unit tests for the new `expected.route` /
  `expected.route_strict` comparison.
- A deliberately drifted fixture (committed under
  `conformance/fixtures/_route_drift_example/`) that the runner
  fails on, demonstrating the safety net.
- The `task conformance:routing-matrix` output is reproducible:
  diffing it across two consecutive runs is empty.

## Done Criteria

- Every fixture under `conformance/fixtures/` has an
  `expected.route` field (or `expected.route_strict: false` plus
  an allowlist).
- The runner fails when a fixture's actual route diverges from its
  expected route.
- `emulatorRoute` is omitted from non-loopback REST responses.
- `task conformance:routing-matrix` runs in CI and the matrix is
  visible as a CI artifact.
