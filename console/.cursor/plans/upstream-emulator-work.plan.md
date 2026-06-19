---
name: Upstream emulator work (parallel track)
overview: Parallel, non-blocking track to file issues and PRs against vantaboard/bigquery-emulator for any BigQuery REST behavior the UI assumes but the emulator lacks. The UI is always built to the real BigQuery contract; this track closes emulator gaps so features work fully against the emulator too.
todos:
  - id: triage
    content: Maintain the gap inventory below; add an entry whenever a UI phase hits an unsupported endpoint/field
    status: pending
  - id: dataset-metadata
    content: Upstream — GET /datasets/{id} full metadata (collation, rounding mode, time travel, labels, tags, replicas)
    status: pending
  - id: table-storage
    content: Upstream — table storage byte stats + view/MV query text + materialized view typing
    status: pending
  - id: schema-patch
    content: Upstream — PATCH /tables/{id} schema updates (mode/description/add fields)
    status: pending
  - id: tabledata
    content: Upstream — GET /tables/{id}/data (tabledata.list) pagination for Preview
    status: pending
  - id: jobs
    content: Upstream — copy/snapshot/load jobs and DDL/DML query jobs (CREATE VIEW/FUNCTION, CTAS)
    status: pending
  - id: routines
    content: Upstream — routines list/get/create endpoints
    status: pending
  - id: external-ingest
    content: Upstream — external source ingestion (GCS/S3/Azure/Drive/Bigtable)
    status: pending
  - id: sql-tools-api
    content: Upstream — SQL Tools API shipped and enabled in default releases (format/parse/complete)
    status: pending
  - id: sql-tools-completion
    content: Upstream — SQL Tools completion depth (routines, in-scope columns, qualified names, diagnostic spans)
    status: pending
  - id: sql-tools-analyze
    content: Upstream — POST /api/emulator/sql/analyze (referenced tables from AST for reference panel)
    status: pending
isProject: false
---

# Upstream emulator work (parallel track)

## Dependencies

- None blocking. Runs alongside all UI milestones.
- Index: [`00-index.plan.md`](00-index.plan.md)
- Roadmap: [`ROADMAP.md`](../../ROADMAP.md) → Upstream emulator work + Phasing → "Upstream emulator work (parallel track)".

## Principle

UI features are **never descoped** because the emulator lacks support. Build to the real BigQuery REST contract; when the emulator returns an error or missing fields, the UI degrades gracefully (placeholder/`—`/inline error) and an entry here drives an issue/PR against [vantaboard/bigquery-emulator](https://github.com/vantaboard/bigquery-emulator).

## Gap inventory

| # | Area | Expected REST / behavior | UI feature blocked | Source plan |
|---|------|---------------------------|--------------------|-------------|
| 1 | Dataset metadata | `GET /datasets/{id}` full fields (labels, tags, replicas, collation, rounding mode, time travel) | Dataset Details tab | m2-01 |
| 2 | Table metadata | Storage byte stats, view/MV query text, materialized-view typing | Table Details tab | m1-01, m2-02 |
| 3 | Schema update | `PATCH /tables/{id}` | Edit Schema modal | m3-02 |
| 4 | Table data | `GET /tables/{id}/data` (`tabledata.list`) pagination | Preview tab | m2-02 |
| 5 | Table create | Table insert + load jobs | Create Table modal | m3-01 |
| 6 | Copy jobs | Copy table / copy dataset | Copy modals | m3-02 |
| 7 | Snapshot jobs | Table snapshot creation | Snapshot modal | m3-02 |
| 8 | Delete | `DELETE` datasets and tables | Delete buttons | m3-02 |
| 9 | Routines | List/get/create routines | Routines + UDF autocomplete | m5-01, m4-01 |
| 10 | DDL/DML jobs | Query jobs for CREATE VIEW / CREATE FUNCTION / CTAS | Save view, routines, save-as | m4-01, m5-01 |
| 11 | Saved queries | Server-backed saved query objects (optional) | Save query (versioned) | m4-01 |
| 12 | Replicas | Cross-region replica metadata | Dataset Details → Replicas | m2-01 |
| 13 | External ingestion | GCS, S3, Azure, Drive, Bigtable sources | Create Table sources | m3-01, m5-01 |
| 14 | SQL Tools API | Opt-in routes `POST /api/emulator/sql/{format,parse,complete}`; gateway flag `--enable-sql-tools-api` | M4 format/lint/completion | m4-01 |
| 15 | SQL Tools — completion | Routines in `/complete`; in-scope column completion; `project.dataset.table` names; diagnostic byte spans | M4/M5 autocompletion | m4-01, m5-01 |
| 16 | SQL Tools — analyze | `POST /api/emulator/sql/analyze` → referenced table paths | M4 reference panel (SQL-inferred) | m4-01 |
| 17 | SQL Tools — ops | Capabilities probe; empty-SQL completion; UTF-8 byte offset contract documented | M4 editor integration, e2e | m4-01 |

## Process

1. When a UI plan hits an unsupported behavior, confirm the real BigQuery contract (shape, params, response).
2. Reproduce the gap against the emulator (minimal request).
3. File an issue on [vantaboard/bigquery-emulator](https://github.com/vantaboard/bigquery-emulator) with the contract reference and repro; open a PR if feasible.
4. Update the relevant row above with the issue/PR link and status; flip the matching todo.
5. Once landed upstream and released, remove the UI's graceful-degradation note for that feature.

## Done criteria

- Every UI graceful-degradation path has a corresponding tracked upstream item.
- Gap inventory stays current as phases progress (no silently hidden UI controls).
