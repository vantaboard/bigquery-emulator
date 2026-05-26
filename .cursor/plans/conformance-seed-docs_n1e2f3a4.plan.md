---
name: conformance-seed-docs
overview: "Phase 8c: seed fixtures, contributor docs, Taskfile targets."
todos:
  - id: seed-fixtures
    content: "Author ≥20 fixtures: SELECT, dryRun errors, insertAll, DML, JOIN, GROUP BY"
    status: completed
  - id: docs-contributing
    content: "Add conformance/README.md: add fixtures, run locally"
    status: completed
  - id: taskfile-target
    content: "Add task conformance:run to Taskfile.yml"
    status: completed
isProject: false
---

# Phase 8c: Conformance Seed Docs

## Prerequisites

- [conformance-diff-ci_m0d1e2f3.plan.md](conformance-diff-ci_m0d1e2f3.plan.md)

## Verification

```bash
task conformance:run
```

## Done criteria

- ≥20 fixtures pass on reference_impl+memory

## Next plan(s)

- [docker-compose-smoke_o2f3a4b5.plan.md](docker-compose-smoke_o2f3a4b5.plan.md)
