---
name: conformance-fixtures-runner
overview: "Phase 8a: fixture layout and conformance runner CLI."
todos:
  - id: fixture-layout
    content: "Add conformance/fixtures/ YAML cases: setup, query, expected rows/error, profile tags"
    status: pending
  - id: runner-cli
    content: "Add conformance/cmd/runner: start emulator or connect, run fixture lifecycle, diff results"
    status: pending
isProject: false
---

# Phase 8a: Conformance Fixtures Runner

## Prerequisites

- [query-select-e2e_b3e4f5a6.plan.md](query-select-e2e_b3e4f5a6.plan.md)

## Verification

```bash
go run ./conformance/cmd/runner --help
```

## Done criteria

- Runner CLI starts and accepts fixture path

## Next plan(s)

- [conformance-diff-ci_m0d1e2f3.plan.md](conformance-diff-ci_m0d1e2f3.plan.md)
