---
name: conformance-diff-ci
overview: "Phase 8b: row diff engine and CI matrix job."
todos:
  - id: diff-engine
    content: "Row diff: order-insensitive aggregates, typed cell comparison, schema-only dryRun mode"
    status: completed
  - id: ci-matrix
    content: "Add .github/workflows/conformance.yml: matrix engine×storage profiles"
    status: completed
isProject: false
---

# Phase 8b: Conformance Diff Ci

## Prerequisites

- [conformance-fixtures-runner_l9c0d1e2.plan.md](conformance-fixtures-runner_l9c0d1e2.plan.md)
- [bootstrap-ci_e0f1a2b3.plan.md](bootstrap-ci_e0f1a2b3.plan.md)

## Verification

```bash
task conformance:run PROFILE=reference_impl,memory || go run ./conformance/cmd/runner
```

## Done criteria

- CI conformance job runs on PR

## Next plan(s)

- [conformance-seed-docs_n1e2f3a4.plan.md](conformance-seed-docs_n1e2f3a4.plan.md)
