---
name: gateway-legacy-sql
overview: "Phase 1b: reject useLegacySql=true, add tests, update docs."
todos:
  - id: use-legacy-sql
    content: "In queries.go QueryRun: if useLegacySql==true return HTTP 400 invalidQuery before engine"
    status: completed
  - id: use-legacy-sql-test
    content: "Add queries_test.go: useLegacySql=true rejected; false/unset passthrough to 501"
    status: completed
  - id: emulator-host-doc
    content: "Verify README BIGQUERY_EMULATOR_HOST; update docs/REST_API.md auth section"
    status: completed
isProject: false
---

# Phase 1b: Gateway Legacy Sql

## Prerequisites

- [gateway-auth-discovery_f1a2b3c4.plan.md](gateway-auth-discovery_f1a2b3c4.plan.md)

## Verification

```bash
go test ./gateway/handlers/... -run Legacy
```

## Done criteria

- useLegacySql=true → 400 invalidQuery
- Tests pass

## Next plan(s)

- [catalog-grpc-cpp_q2f3a4b5.plan.md](catalog-grpc-cpp_q2f3a4b5.plan.md)
