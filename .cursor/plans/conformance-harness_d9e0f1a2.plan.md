---
name: conformance-harness
overview: "Phase 8: build conformance test harness — YAML/JSON query fixtures, REST client runner, result diff against expected rows/schema, CI matrix across engine/storage profiles, and regression gate for merged PRs."
todos:
  - id: fixture-layout
    content: "Add conformance/fixtures/{queries,datasets}/ with YAML cases: setup SQL, query, expected rows or error, profile tags (reference_impl+memory, duckdb+duckdb)"
    status: pending
  - id: runner-cli
    content: "Add conformance/cmd/runner (Go): start emulator subprocess or connect to running instance, run fixture lifecycle (create dataset/table, insertAll, query), diff results"
    status: pending
  - id: diff-engine
    content: "Implement row diff: order-insensitive for aggregates, typed cell comparison per wire rules, schema-only mode for dryRun fixtures"
    status: pending
  - id: ci-matrix
    content: "Add GitHub Actions job conformance.yml: matrix engine×storage profiles, run runner against built binary; fail on regression"
    status: pending
  - id: seed-fixtures
    content: "Author ≥20 fixtures covering SELECT, dryRun errors, insertAll, DML, JOIN, GROUP BY; tag min profile per fixture"
    status: pending
  - id: docs-contributing
    content: "Add conformance/README.md: how to add fixtures, run locally (task conformance or go run), profile flags"
    status: pending
  - id: taskfile-target
    content: "Add task conformance:run and conformance:fix targets to Taskfile.yml"
    status: pending
isProject: false
---

# Phase 8: Conformance harness

## Prerequisites

- [reference-impl-execution_9c0d1e2f.plan.md](reference-impl-execution_9c0d1e2f.plan.md) — at least one working query path
- [bootstrap-ci-docker_e0f1a2b3.plan.md](bootstrap-ci-docker_e0f1a2b3.plan.md) — CI infrastructure

Best run after Plans 07–10 have meaningful coverage; can start skeleton early in parallel with Plan 07.

## Scope

**In:** Fixture format, runner, CI gate, contributor docs.

**Out:** Upstream BigQuery public dataset replay. Performance benchmarks.

## Key files

- `conformance/fixtures/`
- `conformance/cmd/runner/main.go`
- `conformance/internal/diff/diff.go`
- `.github/workflows/conformance.yml`
- `Taskfile.yml`

## Verification

```bash
task conformance:run PROFILE=reference_impl,memory
# expect all fixtures pass
```

## Done criteria

- Runner executes locally and in CI without manual steps.
- Matrix runs reference_impl+memory and duckdb+duckdb (skip if profile not built).
- Adding a new fixture is documented in ≤10 lines.
- PR CI fails on intentional regression (test with broken fixture in branch).

## Next plan

[distribution-release_4b5c6d7e.plan.md](distribution-release_4b5c6d7e.plan.md)
