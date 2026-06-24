---
name: Upstream UI gaps overview
overview: Index of UI inventory gaps #1–17 mapped to emulator plan files, verified REST state at HEAD, and recommended execution order.
todos:
  - id: refresh-gap-map
    content: Keep the gap→plan table below aligned with HEAD verification after each upstream fix lands
    status: completed
  - id: close-verify-only-gaps
    content: Add regression/e2e tests for gaps marked verify-only (4, 6, 7, 8, 14, 16, 17) so they stay closed
    status: completed
isProject: false
---

# Upstream UI gaps — overview & index

**Consumer:** [bigquery-emulator-ui](https://github.com/vantaboard/bigquery-emulator-ui) — a
BigQuery-Console-style UI that talks only through the standard REST surface
(`/bigquery/v2/*`) plus the optional SQL Tools API (`/api/emulator/sql/*`).

**Origin:** gap inventory rows #1–17 from
`../bigquery-emulator-ui/.cursor/plans/upstream-emulator-work.plan.md`.

**Verified at HEAD:** `cf8efc9d` (2026-06-24) via integration e2e + unit
regression suite; SQL Tools spot-checked with `--enable-sql-tools-api`.

**Design principle (do not regress):** the UI is built to the real BigQuery REST
contract. It never removes controls when the emulator lacks support; it shows
errors, `—`, or fallbacks. Closing these gaps in the emulator makes the UI work
end-to-end with **no UI changes**.

---

## Gap → plan → current-state map

| # | Gap (UI brief) | Plan file | Verified state at HEAD | Work needed |
|---|----------------|-----------|------------------------|-------------|
| 1 | Dataset metadata (`GET /datasets/{id}`) | [`07-dataset-table-metadata.plan.md`](07-dataset-table-metadata.plan.md) | **Done** — labels, collation, rounding, time travel, case insensitivity, `resourceTags`, `replicas[]`, timestamps persist | None |
| 2 | Table metadata (storage stats, view/MV typing) | [`07-dataset-table-metadata.plan.md`](07-dataset-table-metadata.plan.md) | **Done** — byte stats stubbed to `"0"`, `view.useLegacySql`, constraints, tags round-trip | None |
| 3 | Schema PATCH persistence | [`02-patch-schema-persistence.plan.md`](02-patch-schema-persistence.plan.md) | **Pass** — e2e + unit locked | Verify only |
| 4 | `tabledata.list` pagination | [`03-tabledata-list-pagination.plan.md`](03-tabledata-list-pagination.plan.md) | **Pass** — multi-page e2e locked | Verify only |
| 5 | Table create / load jobs | [`05-load-jobs-local-ingest.plan.md`](05-load-jobs-local-ingest.plan.md) | **Done** — file/gs/multipart/resumable; `s3://` via `S3_ENDPOINT`; docs in [`load-jobs.md`](../docs/guides/load-jobs.md) | None |
| 6 | Copy table job | [`04-copy-snapshot-jobs.plan.md`](04-copy-snapshot-jobs.plan.md) | **Pass** — copy e2e locked | Verify only |
| 7 | Snapshot jobs | [`04-copy-snapshot-jobs.plan.md`](04-copy-snapshot-jobs.plan.md) | **Pass** — SNAPSHOT e2e locked | Verify only |
| 8 | Delete tables / datasets | [`07-dataset-table-metadata.plan.md`](07-dataset-table-metadata.plan.md) | **Pass** — delete-dataset e2e locked | Verify only |
| 9 | Routines REST + UDF invocation | [`06-routines-api-completeness.plan.md`](06-routines-api-completeness.plan.md) + [`10-routine-qualified-name-resolution.plan.md`](10-routine-qualified-name-resolution.plan.md) | **Done** — list/get/CRUD + qualified `` `proj.ds.fn`(x) `` calls | None |
| 10 | DDL/DML jobs (CREATE VIEW/FUNCTION, CTAS) | [`01-create-view-ddl-visibility.plan.md`](01-create-view-ddl-visibility.plan.md) | **Done** — dotted view list id + `view.query` in list/get | None |
| 11 | Saved queries | _none_ | **N/A** — UI uses `localStorage` | No |
| 12 | Dataset replicas metadata | [`07-dataset-table-metadata.plan.md`](07-dataset-table-metadata.plan.md) | **Pass** — static echo on PATCH/GET | Verify only |
| 13 | External ingestion (GCS/S3/Azure/Drive/Bigtable) | [`11-external-source-ingestion.plan.md`](11-external-source-ingestion.plan.md) | **Done** — Bigtable insert 200; Azure/Drive explicit 400; `s3://` load via `S3_ENDPOINT` | None |
| 14 | SQL Tools API shipped + enabled | [`09-sqltools-completion-analyze.plan.md`](09-sqltools-completion-analyze.plan.md) | **Pass** — capabilities e2e + unit locked | Verify only |
| 15 | SQL Tools completion depth | [`09-sqltools-completion-analyze.plan.md`](09-sqltools-completion-analyze.plan.md) | **Done** — user UDFs as `kind:routine` with `fqn` | None |
| 16 | SQL Tools analyze | [`09-sqltools-completion-analyze.plan.md`](09-sqltools-completion-analyze.plan.md) | **Pass** — analyze e2e locked | Verify only |
| 17 | SQL Tools ops (capabilities, offsets) | [`09-sqltools-completion-analyze.plan.md`](09-sqltools-completion-analyze.plan.md) | **Pass** — offset-zero unit + capabilities e2e | Verify only |

**Out of UI inventory (local dev):** [`08-seed-struct-json-regression.plan.md`](08-seed-struct-json-regression.plan.md) — REPEATED STRUCT seed loader regression guard (e2e locked).

---

## Execution order (completed 2026-06-24)

All planned phases landed:

1. **Plan 01** — CREATE VIEW list id + `view.query` in `tables.list`
2. **Plan 10** — qualified UDF/routine name resolution
3. **Plan 09** — SQL Tools completion (`kind:routine`, `fqn`, column fallback)
4. **Plan 07** — dataset/table metadata completeness
5. **Plan 11** — external source schemes (Bigtable, Azure/Drive errors, `s3://`)
6. **Plan 05** — load-job docs + multipart e2e
7. **Verify batch** — plans 02, 03, 04, 06, 08 e2e/unit regression
8. **Plan 00** — this overview refresh

---

## Shared architecture context (read before any plan)

The gateway splits responsibility between an **in-memory `MetadataStore`**
(REST-only fields the engine does not own) and the **engine gRPC catalog**
(schema, row counts, table existence):

```
INSERT/PATCH/PUT  ->  MetadataStore (labels, type, view, friendlyName, ...)
                   ->  Engine catalog (schema at insert; PATCH schema sync)

GET               ->  Engine DescribeTable  (schema, existence)
                   +  MetadataStore overlay (labels, type, view, ...)
                   +  MergeSchemaPolicyTags (policyTags / collation only)
```

- `gateway/handlers/metadata_store.go` — overlay store and field filters.
- `gateway/handlers/datasets.go`, `tables.go`, `tables_schema.go` — REST handlers.
- `gateway/bqtypes/types.go` — wire types for dataset/table/schema.
- DDL query jobs register objects in the **engine** (`view_registry`, `udf_registry`,
  DuckDB catalog). Gateway mirrors views via `persistViewFromDDL`
  (`gateway/handlers/views_catalog.go`) and routines via `persistRoutineFromDDL`.

## Conventions for these plans

- Each plan has YAML frontmatter (`name`, `overview`, `todos`) plus: current state
  (with file anchors), root cause, UI-observable done-criteria, steps, tests.
- Keep `docs/REST_API.md` and `ROADMAP.md` in sync when a status changes.
- Re-verify with gateway on `:9050` before updating “Verified state at HEAD”.

## Note on Gap 11 (saved queries)

The UI stores saved queries in `localStorage`. Server-side saved-query objects
are not part of the open-source BigQuery REST surface the emulator targets — **no plan**.
