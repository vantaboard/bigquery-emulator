# Upstream UI gaps — overview & index

**Consumer:** [bigquery-emulator-ui](https://github.com/vantaboard/bigquery-emulator-ui) — a
BigQuery-Console-style UI that talks only through the standard REST surface
(`/bigquery/v2/*`) plus the optional SQL Tools API (`/api/emulator/sql/*`).

**Origin of these plans:** a gap brief filed against the published image
`ghcr.io/vantaboard/bigquery-emulator:v0.5.0` (UI gap inventory rows #1–17). The
brief was triaged against the current tree (HEAD `d390572`, branch `main`) —
**not** against v0.5.0 — so several "gaps" are already closed in source and are
captured here as verify/regression-test plans rather than feature work.

**Design principle (do not regress):** the UI is built to the real BigQuery REST
contract. It never removes controls when the emulator lacks support; it shows
errors, `—`, or fallbacks. Closing these gaps in the emulator makes the UI work
end-to-end with **no UI changes**.

---

## Gap → plan → current-state map

| # | Gap (UI brief) | Plan file | Verified state at HEAD | Work needed |
|---|----------------|-----------|------------------------|-------------|
| 10 | CREATE VIEW visible in tables.list/get + `view.query` | [`01-create-view-ddl-visibility.plan.md`](01-create-view-ddl-visibility.plan.md) | **Partial** — DDL runs, `statementType=CREATE_VIEW`; list merges engine view registry but type=`TABLE`; `tables.get` 404; `view.query` not served for DDL views | **Yes (P1)** |
| 3 | PATCH schema persistence reflected on GET | [`02-patch-schema-persistence.plan.md`](02-patch-schema-persistence.plan.md) | **Broken** for mode relax / descriptions / `defaultValueExpression`; field-add works | **Yes (P1)** |
| 4 | `tabledata.list` pagination | [`03-tabledata-list-pagination.plan.md`](03-tabledata-list-pagination.plan.md) | **Works** for native tables (maxResults/pageToken/startIndex); missing selectedFields/formatOptions/etag, view preview, perf, multi-page test | **Polish (P3)** |
| 6 | Copy table job (`configuration.copy`) | [`04-copy-snapshot-jobs.plan.md`](04-copy-snapshot-jobs.plan.md) | **Works** for basic COPY | **Yes (P4)** |
| 7 | Copy dataset (per-table copy orchestration) | [`04-copy-snapshot-jobs.plan.md`](04-copy-snapshot-jobs.plan.md) | **UI-orchestrated** — no single server job; per-table copy works | Verify only |
| 8 | Table snapshot job (`operationType=SNAPSHOT`, time-travel) | [`04-copy-snapshot-jobs.plan.md`](04-copy-snapshot-jobs.plan.md) | **Missing** `operationType` / `destinationExpirationTime`; deleted-table `@epoch` works | **Yes (P4)** |
| 5 | Load jobs + local file ingest + multipart | [`05-load-jobs-local-ingest.plan.md`](05-load-jobs-local-ingest.plan.md) | **Works** for `gs://`/`file://`/abs paths, CSV/JSON/AVRO/PARQUET/ORC, multipart + resumable; no `s3://` | **Doc + optional (P5)** |
| 11 | Routines list/get/create | [`06-routines-api-completeness.plan.md`](06-routines-api-completeness.plan.md) | **Works** (CRUD + discovery); list/catalog-vs-store divergence, missing timestamps on catalog GET | **Polish (P6)** |
| 1 | Dataset metadata completeness | [`07-dataset-table-metadata.plan.md`](07-dataset-table-metadata.plan.md) | **Partial** — core fields round-trip; missing `defaultRoundingMode`, `maxTimeTravelHours`, `isCaseInsensitive`, `tags`, `replicas[]`; synthetic timestamps | **Yes (P7)** |
| 2 | Table metadata richness (storage stats, view/MV typing) | [`07-dataset-table-metadata.plan.md`](07-dataset-table-metadata.plan.md) | **Partial** — only `numRows` computed; physical/logical byte stats, `view.useLegacySql`, `tableConstraints`, `tags` missing | **Yes (P7)** |
| 9 | Delete dataset (`?deleteContents=true`) | [`07-dataset-table-metadata.plan.md`](07-dataset-table-metadata.plan.md) | **Works** (engine + metadata cascade) | Verify only |
| 12 | Seed loader REPEATED STRUCT + JSON bug | [`08-seed-struct-json-regression.plan.md`](08-seed-struct-json-regression.plan.md) | **Fixed** in commit `684aad2` | Regression guard (P8) |
| 13 | SQL Tools completion depth + `/analyze` + diagnostics | [`09-sqltools-completion-analyze.plan.md`](09-sqltools-completion-analyze.plan.md) | **Works** (6 endpoints + offsets); in-scope columns best-effort, 3-part names partial | **Polish (P9)** |
| 14 | Server-backed saved queries | _none_ | **Out of scope** — UI uses `localStorage`; not standard open-source emulator REST. See note below. | No |

---

## Recommended execution order

Following the UI brief's own priority, ordered by UI impact:

1. **Gap 10** — CREATE VIEW visibility — blocks "Save view" + view resource pages.
2. **Gap 3** — PATCH schema persistence — Edit Schema modal refresh bug.
3. **Gap 4** — `tabledata.list` polish — native Preview path (works today; harden).
4. **Gaps 6/7/8** — copy/snapshot jobs — Copy/Snapshot modals.
5. **Gap 5** — load jobs — Create Table from external/upload sources.
6. **Gap 11** — routines completeness — M5 + completion.
7. **Gaps 1/2** — dataset/table metadata — Details tabs show real data.
8. **Gap 12** — seed regression guard — local dev ergonomics (already fixed).
9. **Gap 13** — SQL Tools completion/analyze polish — editor UX.

## Shared architecture context (read before any plan)

The gateway splits responsibility between an **in-memory `MetadataStore`**
(REST-only fields the engine does not own) and the **engine gRPC catalog**
(schema, row counts, table existence):

```
INSERT/PATCH/PUT  ->  MetadataStore (labels, type, view, friendlyName, ...)
                   ->  Engine catalog (schema at insert; PATCH adds fields only)

GET               ->  Engine DescribeTable  (schema, existence)
                   +  MetadataStore overlay (labels, type, view, ...)
                   +  MergeSchemaPolicyTags (policyTags / collation only)
```

- `gateway/handlers/metadata_store.go` — overlay store and the
  `stripEngineOwnedDatasetFields` / `stripEngineOwnedTableFields` filters.
- `gateway/handlers/datasets.go`, `tables.go`, `tables_schema.go` — REST handlers.
- `gateway/bqtypes/types.go` — wire types for dataset/table/schema.
- This split is the root cause of **Gap 3** (mode reversion) and **Gap 2**
  (storage-metric gaps): GET re-derives schema/stats from the engine and only
  overlays a narrow set of REST-only fields.

DDL executed through query/job paths registers objects in the **engine**
(`view_registry`, `udf_registry`, DuckDB catalog) but the gateway only mirrors
some of them back into `MetadataStore` (routines + models do; **views do not** —
that is Gap 10).

## Conventions for these plans

- Each plan has: current state (with `file:line` anchors), root cause, UI-observable
  done-criteria, file-by-file steps, tests, and out-of-scope notes.
- Keep `docs/REST_API.md` and the "Gateway HTTP surface" section of `ROADMAP.md`
  in sync when a status changes.
- Follow repo rules: auto-commit per logical unit, run `task lint:fix` +
  `task lint:run` before commits touching Go/C++, and honor the bazel/process
  hygiene rules when building or running the engine.

## Note on Gap 14 (saved queries)

The UI stores saved queries in `localStorage`. Server-side saved-query objects
are not part of the open-source BigQuery REST surface the emulator targets, so
there is **no plan**. Revisit only if a future emulator feature adds parity.
