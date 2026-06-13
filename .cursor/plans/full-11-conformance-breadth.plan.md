---
name: Full 11 — Conformance breadth
overview: Grow the regression net to match the now-much-larger supported surface - widen the GoogleSQL .test corpus lane well beyond the single logical_functions.test starter file, and systematically shrink the third-party skip matrices as the full-01..10 plans land shapes - so the emulator's coverage is provable, not asserted.
est_effort: ~1-2 weeks (ongoing, runs alongside the others)
isProject: true
todos:
  - id: corpus-expansion
    content: "Widen conformance/googlesql-corpus/: vendor additional .test files beyond logical_functions.test (string, math, array, date/time, aggregation, analytic) per conformance/googlesql-corpus/README.md; gate each new file behind the runner so only known-passing cases are pinned and genuinely-unsupported cases are explicitly excluded (not silently skipped)."
    status: pending
  - id: corpus-ci
    content: "Extend the googlesql-corpus CI job (.github/workflows/googlesql-parity.yml) to run the widened subset; keep it non-flaky (deterministic ordering, no public-data dependencies)."
    status: pending
  - id: skip-matrix-audit
    content: "Audit every third_party/*/emulator_*skip* matrix entry against what full-01..10 landed: remove rows whose blocking shape now works (info_schema/catalog -> 01, numeric -> 02, geography -> 06, grant_access/column_policy -> 07, functions/test_js -> 05, wildcard -> 04) and re-run each suite to prove the removal."
    status: pending
  - id: fixture-route-backfill
    content: "Backfill expected.route labels on any new fixtures the full-* plans added so task conformance:routing-matrix stays a complete snapshot; confirm no fixture silently drifted route."
    status: pending
  - id: coverage-publish
    content: "Confirm the coverage publisher (taskfiles/coverage.yml + .github/workflows/coverage-publish.yml) ingests the widened fixture + corpus counts; update any hardcoded fixture-count references in docs (ROADMAP says 160+ now)."
    status: pending
---

# Full 11 — Conformance breadth

## Why

The supported surface roughly doubles across full-01..10, but the
regression net hasn't grown with it. The GoogleSQL `.test` corpus lane
still pins only `logical_functions.test` (55 cases), and the third-party
skip matrices still carry rows for shapes that the earlier plans land.
Coverage should be *provable*.

## Key files

- [`conformance/googlesql-corpus/`](../../conformance/googlesql-corpus/) — corpus runner + vendored `.test` subset + README
- `.github/workflows/googlesql-parity.yml` — corpus CI job
- `.github/workflows/conformance.yml` — fixture lane + routing matrix artifact
- `third_party/*/emulator_*skip*`, `third_party/README.md` — skip matrices
- `taskfiles/coverage.yml`, `.github/workflows/coverage-publish.yml` — coverage publish

## Steps

1. Vendor + gate additional `.test` corpus files (one category at a
   time; pin only known-passing cases, explicitly exclude unsupported).
2. Wire the widened subset into CI.
3. Sweep the skip matrices against landed shapes; remove + prove.
4. Backfill `expected.route` on new fixtures.
5. Confirm coverage publish picks up the new counts; fix doc counts.

## Sequencing

This plan's *authoring* (vendoring `.test` files, editing CI YAML, skip
matrices) is engine-change-free and can run in the background alongside
one engine-lane plan. Its *execution* steps (running the corpus via
`jobs.query`, re-running third-party suites) must wait for a bazel-quiet
window — `task bazel:status` -> `(clean)` and `free -h` > 4 GiB — exactly
like parity plan 13.

## Verify

```bash
task conformance:googlesql-corpus     # widened subset, all pinned cases pass
task conformance:run
task conformance:routing-matrix       # complete, no drift
# each suite whose skip rows you removed:
task thirdparty:golang ; task thirdparty:python ; task thirdparty:dbt-bigquery-tests
```

## Out of scope

- Pinning corpus cases for deliberately-`unsupported` families (those
  stay excluded with a comment, not skipped silently).
- New first-party fixtures for shapes their own full-* plan already
  pinned (this plan is breadth/regression-net, not feature work).
