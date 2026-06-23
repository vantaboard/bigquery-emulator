---
name: SQL Tools completion and analyze
overview: SQL Tools API endpoints work; polish completion for user routines, qualified names, and diagnostic offset edge cases.
todos:
  - id: user-routines-in-complete
    content: Emit DDL-created routines with kind:routine, fqn, and signature detail in /complete
    status: pending
  - id: qualified-name-candidates
    content: Project-qualified table/dataset/routine candidates for 3-part typing
    status: pending
  - id: in-scope-column-fallback
    content: Heuristic FROM-table column completion when analyze fails on incomplete SQL
    status: pending
  - id: diagnostic-offset-zero
    content: Fix omitempty dropping startByte:0 in diagnostic JSON
    status: pending
  - id: e2e-sqltools-regression
    content: gateway/sqltools tests — capabilities, analyze referencedTables, complete with user UDF
    status: pending
isProject: false
---

# 09 — SQL Tools API completion depth, /analyze, diagnostics polish

- **UI gaps:** #14 (API enablement), #15 (completion), #16 (analyze), #17 (ops)
- **Priority:** #14/#16/#17 **verify-only**; #15 **partial polish (P9)**
- **Verified state at HEAD (`60d19b3e`):** All six endpoints 200 with `--enable-sql-tools-api`.

## Enablement & access (#14 / #17 — verify only)

Verified 2026-06-23: `GET /api/emulator/sql/capabilities` returns expected shape;
POST endpoints 200 with engine attached.

- Gate flag `--enable-sql-tools-api` (`binaries/gateway_main/cli.go` ~267–268;
  server wiring `gateway/server.go` ~255–267). Disabled → 404.
- Remote access flag `--sql-tools-api-allow-remote` enforced in
  `gateway/sqltools/access.go` (`CheckAccess` ~18–31): non-loopback → 403 unless
  the flag is set; optional token via header
  `X-BigQuery-Emulator-SqlTools-Token` / env `BIGQUERY_EMULATOR_SQL_TOOLS_TOKEN`.
- Capabilities response (`handler.go` ~337–356) already matches the UI's
  expected shape:
  ```json
  { "sqlTools": true, "version": "1.0",
    "endpoints": ["format","parse","tokenize","complete","analyze","capabilities"],
    "offsetUnits": ["utf8","utf16"] }
  ```
- POST endpoints return **503** when the gateway runs without the engine
  subprocess (`handler.go` ~88–97). Docker injects the flag via
  `docker/gateway_main.sh`.

**Action:** none for behavior. Document that the token is optional even with
`--allow-remote`, and that POST endpoints need the engine (capabilities works
without it).

## Gap #15 — `/complete` depth (remaining work)

Verified 2026-06-23: user-created UDF `add_two` appears in `/complete` but as
`kind: function` without `fqn` (should be `kind: routine` per `catalog_names.cc`
convention for catalog routines).

Engine: `backend/sqltools/sql_tools_complete.cc` + `catalog_names.cc`
(catalog population) + `sql_references.cc` (analyze-driven scope), orchestrated
in `frontend/handlers/sqltools.cc` (~146–232). Gateway wire/offsets:
`gateway/sqltools/handler.go` (~263–335) + `offsets.go`.

| Candidate source | State | Gap |
|------------------|-------|-----|
| builtin function names (kind `function`) | ✅ | — |
| catalog routine/function names (kind `routine`) | ⚠️ | User DDL routines merged as `function`; missing `fqn` |
| table/view names after `FROM` | ✅ | — |
| in-scope **columns** after `SELECT ... FROM table` | ⚠️ partial | requires a successful `AnalyzeSqlText`; incomplete/invalid SQL → analyze fails silently → no columns |
| qualified `project.dataset.table` | ⚠️ partial | tables/routines carry `label=dataset.x` + `fqn=project.dataset.x`; datasets list dataset-id only (no project-qualified dataset candidates) |

**Steps:**
1. **Harden in-scope columns when analyze fails.** In the Complete handler
   (`frontend/handlers/sqltools.cc` ~193–215 →
   `PopulateInScopeTablesFromAnalyze` / `AppendInScopeColumnCandidates`), add a
   parse-token / heuristic fallback that extracts `FROM <table>` (and aliases)
   from the partial SQL and offers that table's columns even when full analysis
   fails. This is the highest-value editor improvement.
2. **Project-qualified names.** Emit explicit `project.dataset.table` table
   candidates and project-qualified dataset candidates so 3-part typing
   autocompletes (`catalog_names.cc` ~80–98, prefix match ~118–137).
3. **Richer candidate metadata.** Populate `detail`/`kind` for routines beyond
   `SQL`/`routine` (signature summary, return type) so the UI shows useful hints.

## Gap #16 — `/analyze` (verify only)

Verified 2026-06-23: `POST /analyze` with `` SELECT * FROM `local-project.ds.t` ``
returns `referencedTables` + `statementKinds`. Unqualified single-part names may
fail analyze if table is not in default dataset scope.

Exists (`handler.go` ~380–437; `frontend/handlers/sqltools.cc` ~235–295) and
returns `referencedTables[]` (`projectId`/`datasetId`/`tableId`/`alias`/`kind`),
`statementKinds[]`, and `diagnostics[]`. Requires `projectId` + non-empty `sql`.

**Steps:**
1. Note/align that `statementKinds` uses **resolved** node names (e.g.
   `QueryStatement`) which may differ from `/parse` AST names — document the
   distinction (UI brief lists `["SELECT"]`; reconcile naming or document the
   mapping).
2. `referencedTables` only populate from a successful full-statement analyze —
   acceptable; document.

## Gap 13d — `/parse` & `/tokenize` diagnostics (already done; edge cases)

C++ spans (`backend/sqltools/sql_tools.cc` ~76–135): always `line`/`column`/
`message`/`severity`; when SQL present also `startByte`/`endByte`/`endLine`/
`endColumn`. Gateway (`offsets.go` ~75–130) passes byte + line/column spans and,
when `offsetUnit=utf16`, adds `startUtf16`/`endUtf16`.

**Steps:**
1. **`omitempty` drops `startByte:0`.** Audit the diagnostic/token wire structs:
   a legitimate offset `0` is currently elided by `omitempty`, which the UI's
   CodeMirror lint mapping may misread. Use pointer/`*int` or a sentinel so
   position-0 spans serialize.
2. **Consistent empty-`sql` 400.** `/parse` has no gateway-side empty-`sql`
   check (engine returns 400); add a gateway check for a consistent error shape
   across endpoints.
3. Consider always emitting `startUtf16`/`endUtf16` when `offsetUnit=utf16` even
   at position 0.

## Offsets (already correct)

`gateway/sqltools/offsets.go` converts UTF-16 code units ↔ UTF-8 bytes
(`utf16.Encode`/`Decode`), default `utf8`; tested with non-ASCII in
`offsets_test.go`. No change needed beyond the `omitempty` fix above.

## Tests

- `backend/sqltools` (`sql_tools_test.cc`): column completion on **incomplete**
  SQL (post-fallback); project-qualified table candidates.
- `gateway/sqltools` (`*_test.go`): diagnostic with `startByte:0` survives JSON;
  empty-`sql` → 400 for `/parse`; UTF-16 fields present at offset 0.

## Out of scope

- Full semantic analysis parity with production BigQuery.
- New SQL Tools endpoints beyond the documented six.

## Touch list

`backend/sqltools/sql_tools_complete.cc`, `catalog_names.cc`,
`frontend/handlers/sqltools.cc`, `gateway/sqltools/offsets.go`,
`gateway/sqltools/handler.go`, `docs/SQL_TOOLS_API.md`.
