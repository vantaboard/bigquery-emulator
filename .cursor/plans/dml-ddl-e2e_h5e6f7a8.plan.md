---
name: dml-ddl-e2e
overview: "Phase 6d: dmlStats response wiring and full DML/DDL E2E suite."
todos:
  - id: dmlstats-response
    content: "Populate bqtypes.DmlStats on jobs.query for DML statements"
    status: pending
  - id: dml-e2e
    content: "E2E: INSERT→SELECT count; UPDATE verify; DELETE; CTAS; insertAll still works"
    status: pending
isProject: false
---

# Phase 6d: Dml Ddl E2E

## Prerequisites

- [ddl-statements_g4d5e6f7.plan.md](ddl-statements_g4d5e6f7.plan.md)

## Verification

```bash
go test -tags=integration ./gateway/e2e/... -run DMLDDL
```

## Done criteria

- Full DML/DDL E2E suite passes

## Next plan(s)

- [storage-read-proto_i6f7a8b9.plan.md](storage-read-proto_i6f7a8b9.plan.md)
