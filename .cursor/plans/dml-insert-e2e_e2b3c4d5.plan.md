---
name: dml-insert
overview: "Phase 6a: statement classification and INSERT DML on reference impl."
todos:
  - id: dml-analyzer
    content: "Classify SELECT vs DML vs DDL from analyzed AST; UNIMPLEMENTED for unsupported kinds"
    status: completed
  - id: insert-dml
    content: "Reference impl INSERT INTO VALUES/SELECT; update Storage AppendRows; return dmlStats"
    status: completed
isProject: false
---

# Phase 6a: Dml Insert

## Prerequisites

- [query-select-e2e_b3e4f5a6.plan.md](query-select-e2e_b3e4f5a6.plan.md)

## Verification

```bash
curl -X POST localhost:9050/bigquery/v2/projects/test/queries -d '{"query":"INSERT INTO ds.t VALUES(1)","useLegacySql":false}'
```

## Done criteria

- INSERT returns dmlStats.insertedRowCount

## Next plan(s)

- [dml-update-delete_f3c4d5e6.plan.md](dml-update-delete_f3c4d5e6.plan.md)
