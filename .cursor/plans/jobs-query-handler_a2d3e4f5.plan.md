---
name: jobs-query-handler
overview: "Phase 5d: Wire non-dryRun jobs.query and in-memory job registry."
todos:
  - id: go-query-handler
    content: "Wire queries.go non-dryRun: call ExecuteQuery stream, assemble QueryResponse with rows and jobReference"
    status: completed
  - id: job-lifecycle
    content: "Add gateway/jobs/registry.go: generate jobId, store DONE Job with statistics timestamps"
    status: completed
isProject: false
---

# Phase 5d: Jobs Query Handler

## Prerequisites

- [wire-marshal-go_z1c2d3e4.plan.md](wire-marshal-go_z1c2d3e4.plan.md)

## Verification

```bash
curl -X POST localhost:9050/bigquery/v2/projects/test/queries -d '{"query":"SELECT 1","useLegacySql":false}'
```

## Done criteria

- jobs.query returns rows and jobReference

## Next plan(s)

- [query-select-e2e_b3e4f5a6.plan.md](query-select-e2e_b3e4f5a6.plan.md)
