---
name: Upstream UI gaps overview
overview: Index of UI inventory gaps #1–17 mapped to emulator plan files, verified REST state at HEAD, and recommended execution order.
todos:
  - id: refresh-gap-map
    content: Keep the gap→plan table below aligned with HEAD verification after each upstream fix lands
    status: pending
  - id: close-verify-only-gaps
    content: Add regression/e2e tests for gaps marked verify-only (4, 6, 7, 8, 14, 16, 17) so they stay closed
    status: pending
isProject: false
---

# Upstream UI gaps — overview & index

**Consumer:** [bigquery-emulator-ui](https://github.com/vantaboard/bigquery-emulator-ui) — a
BigQuery-Console-style UI that talks only through the standard REST surface
(`/bigquery/v2/*`) plus the optional SQL Tools API (`/api/emulator/sql/*`).

**Origin:** gap inventory rows #1–17 from
`../bigquery-emulator-ui/.cursor/plans/upstream-emulator-work.plan.md`.

**Verified at HEAD:** `60d19b3e` (2026-06-23) via gateway on `:9050` with
`--enable-sql-tools-api`. Minimal REST repros only (no UI).

**Design principle (do not regress):** the UI is built to the real BigQuery REST
contract. It never removes controls when the emulator lacks support; it shows
errors, `—`, or fallbacks. Closing these gaps in the emulator makes the UI work
end-to-end with **no UI changes**.

---

## Gap → plan → current-state map

| # | Gap (UI brief) | Plan file | Verified state at HEAD | Work needed |
|---|----------------|-----------|------------------------|-------------|
| 1 | Dataset metadata (`GET /datasets/{id}`) | [`07-dataset-table-metadata.plan.md`](07-dataset-table-metadata.plan.md) | **Partial** — `location`, `labels`, `defaultCollation`, `defaultRoundingMode`, `maxTimeTravelHours`, `isCaseInsensitive`, `resourceTags`, `replicas[]` round-trip; UI brief “tags” = wire `resourceTags`; `creationTime`/`lastModifiedTime` stable on GET but freshly minted on insert | Polish (P7) |
| 2 | Table metadata (storage stats, view/MV typing) | [`07-dataset-table-metadata.plan.md`](07-dataset-table-metadata.plan.md) | **Partial** — `numRows`/`numBytes`/`numLongTermBytes` present (often `0`); DDL views serve `view.query` on GET; missing `view.useLegacySql`, table `resourceTags`, `tableConstraints`, richer byte breakdown | Yes (P7) |
| 3 | Schema PATCH persistence | [`02-patch-schema-persistence.plan.md`](02-patch-schema-persistence.plan.md) | **Pass** — REQUIRED→NULLABLE, descriptions, `defaultValueExpression` persist; NULLABLE→REQUIRED → 400 (matches BQ) | Verify only |
| 4 | `tabledata.list` pagination | [`03-tabledata-list-pagination.plan.md`](03-tabledata-list-pagination.plan.md) | **Pass** — `maxResults`/`pageToken`/`startIndex`, `totalRows`, `etag`; multi-page works | Verify / polish (P3) |
| 5 | Table create / load jobs | [`05-load-jobs-local-ingest.plan.md`](05-load-jobs-local-ingest.plan.md) | **Partial** — empty insert + CTAS OK; `gs://` load submits (fetch fails without storage emulator); `s3://` unsupported; multipart endpoint exists | Doc + optional schemes (P5) |
| 6 | Copy table job | [`04-copy-snapshot-jobs.plan.md`](04-copy-snapshot-jobs.plan.md) | **Pass** — basic `configuration.copy` → `DONE` | Verify only |
| 7 | Snapshot jobs | [`04-copy-snapshot-jobs.plan.md`](04-copy-snapshot-jobs.plan.md) | **Pass** — `operationType=SNAPSHOT`, `destinationExpirationTime`, destination `type=SNAPSHOT` | Verify only |
| 8 | Delete tables / datasets | [`07-dataset-table-metadata.plan.md`](07-dataset-table-metadata.plan.md) | **Pass** — `DELETE` table 200; `DELETE ?deleteContents=true` cascades | Verify only |
| 9 | Routines REST + UDF invocation | [`06-routines-api-completeness.plan.md`](06-routines-api-completeness.plan.md) + [`10-routine-qualified-name-resolution.plan.md`](10-routine-qualified-name-resolution.plan.md) | **Partial** — list/get/create OK with timestamps; **qualified** `` `proj.ds.fn`(x) `` → 400 “Function not found”; unqualified `SELECT fn(x)` OK | Yes (P2) — plan 10 |
| 10 | DDL/DML jobs (CREATE VIEW/FUNCTION, CTAS) | [`01-create-view-ddl-visibility.plan.md`](01-create-view-ddl-visibility.plan.md) + plan 10 | **Partial** — three-segment `` `p`.`d`.`v` `` CREATE VIEW lists + GET with `view.query`; **dotted** `` `p.d.v` `` registers wrong list id; `view.query` null in list entries | Yes (P1) — plan 01 |
| 11 | Saved queries | _none_ | **N/A** — UI uses `localStorage`; not standard emulator REST | No |
| 12 | Dataset replicas metadata | [`07-dataset-table-metadata.plan.md`](07-dataset-table-metadata.plan.md) | **Pass** — `replicas[]` echoes on PATCH/GET (static; no live replication) | Verify only |
| 13 | External ingestion (GCS/S3/Azure/Drive/Bigtable) | [`11-external-source-ingestion.plan.md`](11-external-source-ingestion.plan.md) | **Open** — Bigtable external insert 400 (`unsupported sourceUri scheme: https`); `s3://` load unsupported; real cloud URIs not end-to-end | Yes (P5) |
| 14 | SQL Tools API shipped + enabled | [`09-sqltools-completion-analyze.plan.md`](09-sqltools-completion-analyze.plan.md) | **Pass** — format/parse/complete/capabilities/analyze all 200 with `--enable-sql-tools-api` | Verify only |
| 15 | SQL Tools completion depth | [`09-sqltools-completion-analyze.plan.md`](09-sqltools-completion-analyze.plan.md) | **Partial** — builtins + catalog tables OK; user-created UDFs appear as `kind:function` not `routine`, no `fqn`; 3-part qualified names partial | Polish (P9) |
| 16 | SQL Tools analyze | [`09-sqltools-completion-analyze.plan.md`](09-sqltools-completion-analyze.plan.md) | **Pass** — `referencedTables` returned when SQL analyzes cleanly (use qualified table names for unambiguous refs) | Verify only |
| 17 | SQL Tools ops (capabilities, offsets) | [`09-sqltools-completion-analyze.plan.md`](09-sqltools-completion-analyze.plan.md) | **Pass** — capabilities probe OK; UTF-8/UTF-16 offset contract documented | Verify only |

**Out of UI inventory (local dev):** [`08-seed-struct-json-regression.plan.md`](08-seed-struct-json-regression.plan.md) — REPEATED STRUCT seed loader regression guard (fixed; keep tests).

---

## Recommended execution order

Ordered by remaining UI impact after HEAD verification:

1. **Gap 10 / plan 01** — dotted CREATE VIEW list id + populate `view.query` in `tables.list`.
2. **Gap 9 / plan 10** — qualified UDF/routine name resolution in queries (`SELECT \`p.d.fn\`(x)`).
3. **Gap 15 / plan 09** — user routines in `/complete` with `kind:routine` + FQN metadata.
4. **Gap 2 / plan 07** — table metadata richness (`view.useLegacySql`, byte stats, constraints).
5. **Gap 13 / plan 11** — external source schemes (Bigtable URI, Azure, Drive, `s3://`).
6. **Gap 5 / plan 05** — load-job docs + optional scheme support (overlaps plan 11).
7. **Gap 1 / plan 07** — dataset timestamp persistence polish on insert.
8. **Verify-only gaps** — 3, 4, 6, 7, 8, 12, 14, 16, 17: add/keep e2e regression tests.

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
