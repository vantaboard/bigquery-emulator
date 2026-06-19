---
name: M4-01 — Query workspace upgrades
overview: Upgrade the query editor with emulator SQL Tools API integration (format/parse/complete) for parser-based completion and diagnostics, REST-backed fallback when SQL Tools is unavailable, a reference side panel, and saving queries (Save query, Save query Classic, Save view via DDL, Save as table). Builds on the multi-tab workspace from M1-03.
todos:
  - id: sqltools-client
    content: Add typed sqlTools client (format/parse/complete/capabilities probe) and Vite+nginx proxy for /api/emulator/sql/*
    status: pending
  - id: editor-integration
    content: Wire SqlEditor CodeMirror extensions — completion via /complete, lint via debounced /parse, format via /format with sql-formatter fallback
    status: pending
  - id: catalog-fallback
    content: Slim REST catalog fallback (@codemirror/lang-sql schema from explorerQueries) when SQL Tools is disabled or against real BigQuery
    status: pending
  - id: diagnostics
    content: Map SqlDiagnostic spans to CodeMirror lint (UTF-8 byte offset ↔ editor position conversion)
    status: pending
  - id: reference-panel
    content: Add reference side panel — REST schema for tab-bound resource; infer tables from SQL when upstream /analyze lands
    status: pending
  - id: tools-menu
    content: Add Tools menu toggles — emulator parser vs client fallback, strict vs lenient format
    status: pending
  - id: saving
    content: Add Save dropdown — Save query, Save query (Classic), Save view (DDL), Save as table
    status: pending
  - id: e2e
    content: Add Playwright coverage for autocompletion suggestions and saving a view (enable SQL Tools in e2e compose when release ships)
    status: pending
isProject: false
---

# M4-01: Query workspace upgrades

## Dependencies

- M1 plans (especially [`m1-03-routing-workspace-breadcrumbs.plan.md`](m1-03-routing-workspace-breadcrumbs.plan.md) for multi-tab query tabs and [`m1-01-api-and-types.plan.md`](m1-01-api-and-types.plan.md) for REST catalog/job APIs).
- Upcoming [bigquery-emulator SQL Tools API](https://github.com/vantaboard/bigquery-emulator/blob/main/docs/SQL_TOOLS_API.md) (`POST /api/emulator/sql/{format,parse,complete}`), enabled with `--enable-sql-tools-api`.
- Index: [`00-index.plan.md`](00-index.plan.md)
- Roadmap: [`ROADMAP.md`](../../ROADMAP.md) → Query workspace + Saving queries.
- Upstream gaps: [`upstream-emulator-work.plan.md`](upstream-emulator-work.plan.md) → SQL Tools API rows.

## Goal

Make the editor console-grade: smart completion, inline errors, a schema reference panel, and the Save menu — all per query tab. Prefer the emulator SQL Tools API when available; degrade to client-side `@codemirror/lang-sql` + `sql-formatter` otherwise.

## Current state

- [SqlEditor.tsx](../../src/features/explorer/components/SqlEditor.tsx): CodeMirror with `sql()` extension, no schema completion, no diagnostics.
- Run + Format already implemented (carried into per-tab `QueryTab` by M1-03); Format uses client `sql-formatter`.
- `@codemirror/lang-sql` and `sql-formatter` already in [package.json](../../package.json).
- Vite proxies `/bigquery` only ([vite.config.ts](../../vite.config.ts)); SQL Tools routes not proxied yet.

## Scope

### SQL Tools client — [src/lib/sqlTools.ts](../../src/lib/sqlTools.ts) (new)

Typed wrappers mirroring emulator shapes:

- `format`, `parse`, `complete` → `POST /api/emulator/sql/{format,parse,complete}`
- `probeCapabilities()` — try a lightweight request (or future `GET /capabilities`) to detect whether SQL Tools is enabled; cache result per session.
- Pass `projectId` and optional `defaultDatasetId` from the active query tab into `/complete`.
- Optional `X-BigQuery-Emulator-SqlTools-Token` header when `VITE_SQL_TOOLS_TOKEN` is set (remote/LAN/Compose).

### Dev and production proxy

- [vite.config.ts](../../vite.config.ts): proxy `/api/emulator` → `VITE_PROXY_TARGET` (same as `/bigquery`).
- [nginx.conf](../../nginx.conf): proxy `/api/emulator/` → `bigquery:9050` in Docker.
- [docker-compose.yaml](../../docker-compose.yaml): document `--enable-sql-tools-api` and `--sql-tools-api-allow-remote` on the emulator service once the release image includes SQL Tools (required for Compose because nginx is not loopback).

### Auto-completion

Primary path (SQL Tools enabled):

- CodeMirror completion source calls `/complete` with `sql`, `cursorByteOffset`, `projectId`, `defaultDatasetId`.
- Map `candidates`, `replacementStart`, `replacementEnd` into CodeMirror completions; convert UTF-8 byte offsets to editor positions.
- Debounce requests (~150ms); cancel in-flight on cursor move.

Fallback path (real BigQuery or SQL Tools disabled):

- Lazy REST catalog via `explorerQueries` (projects → datasets → tables/views → columns, routines when available).
- Configure `@codemirror/lang-sql` `schema` plus a custom source for `project.dataset.table` and UDF names.

### Diagnostics

- CodeMirror lint source debouncing `/parse` (~400ms) on SQL Tools path.
- Map `diagnostics[]` to lint markers; convert line/column (and byte spans when upstream adds `startByte`/`endByte`).
- Non-blocking; mirror console "Syntax error" gutter feedback.

### Format

- When SQL Tools is available, **Format** calls `/format` (lenient by default; Tools menu can toggle `strict`).
- Fall back to `sql-formatter` on error or when SQL Tools is disabled.

### Reference side panel

- Toggleable panel showing field names/types for the table/view referenced while editing.
- **Tab-bound resource**: when the query tab has `projectId` / `datasetId` / `tableId`, fetch schema via `GET .../tables/{tableId}`.
- **SQL-inferred tables** (when upstream lands `POST /api/emulator/sql/analyze`): show schemas for tables referenced in the active SQL text.

### Tools menu

- Toggle **Emulator parser** (SQL Tools) vs **Client fallback** (persist preference in workspace UI store).
- Toggle **Strict format** (`strict: true` on `/format`).

### Saving queries — Save dropdown (per query tab)

- **Save query** — versioned saved query stored in `localStorage` (extends the M1-03 workspace store); upstream entry for server-backed saved queries.
- **Save query (Classic)** — simple saved query, no version history.
- **Save view** — `CREATE VIEW`/`CREATE OR REPLACE VIEW` DDL via `submitJob`/query; refresh tree. Use `/parse` `statementKinds` to validate SELECT vs DDL when SQL Tools is enabled.
- **Save as...** — save results to a table via `CREATE TABLE AS SELECT` / load job.

## Out of scope

- Routine creation UI (M5) — completion references routines via SQL Tools when upstream adds them.
- External source ingestion (M5).
- Implementing SQL Tools endpoints in this repo (upstream emulator only).

## Verification

```bash
npm run build
npm run lint
npm run test:e2e
```

Manual: with SQL Tools enabled on the emulator, type a query and get table/column suggestions via `/complete`; introduce a syntax error and see a `/parse` diagnostic; Format uses emulator formatter; toggle Tools menu fallback and confirm client path still works; save a query (both kinds) and reload; save a view and see it in the tree.

## Done criteria

- Completion suggests datasets/tables/columns/keywords/functions via `/complete` when SQL Tools is enabled; REST fallback when not.
- Syntax diagnostics show inline from `/parse` (or client lint absent SQL Tools).
- Format uses `/format` when SQL Tools is enabled; `sql-formatter` fallback otherwise.
- Reference panel shows schema for tab-bound resource; ready for `/analyze` when upstream ships.
- Tools menu toggles parser source and strict format.
- Save query / Save query (Classic) persist to the browser and restore on reload; Save view creates a view; Save as creates a table.
