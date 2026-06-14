---
name: Expand 01 — External data sources (fixture + opt-in live)
overview: Close the external-data-source gap from ROADMAP §Planned work. Today the emulator handles local file:// LOAD/EXPORT and GCS-backed external tables via fake-gcs; gs:// ingest/export DDL, Google Sheets external tables, and federated/connection-backed scans surface UNIMPLEMENTED or 501. Add a per-source configuration model so each external source resolves to either deterministic fixture/local data or an opt-in live upstream (a real Google Sheets doc, a real GCS bucket, a federated Cloud SQL/Spanner endpoint), without routing query execution through the real BigQuery service.
est_effort: ~3-4 weeks
isProject: true
todos:
  - id: config-model
    content: "Design + implement the per-source configuration model: a mapping from an external source identity (external table URI, connection id, Sheets docId, federated endpoint) to a resolution mode (`fixture` | `local` | `live`). Decide the surface (gateway config file under --data_dir, env vars, and/or a REST-visible connection resource). Default to fixture/local; `live` is strictly opt-in and credentials-gated."
    status: pending
  - id: gcs-load-export
    content: "gs:// for LOAD DATA / EXPORT DATA. In fixture/local mode, resolve gs:// URIs against fake-gcs (STORAGE_EMULATOR_HOST) the same way external tables already do, and/or a local snapshot dir under --data_dir. In live mode, fetch/write the real bucket with opt-in credentials. Promote the `LOAD DATA <gs://...>` row off `unsupported` in ENGINE_POLICY; reuse RunLoadData / EXPORT COPY paths."
    status: pending
  - id: google-sheets
    content: "Google Sheets external tables (GOOGLE_SHEETS / googleSheetsOptions): replace the gateway 501 with (fixture/local) a locally-stored sheet snapshot (CSV/JSON under --data_dir or a conformance fixture) materialized into the engine catalog at insert time, and (live) an opt-in Google Sheets fetch. Round-trip externalDataConfiguration through MetadataStore as today. Live-upstream integration target: the public Google sample sheet docId 1BxiMVs0XRA5nFMdKvBdBZjgmUUqptlbs74OgvE2upms (\"Example Spreadsheet\", tab \"Class Data\") — because it is link-public, the live fetch can use the unauthenticated CSV export (https://docs.google.com/spreadsheets/d/<id>/export?format=csv&gid=<gid>) for the read path; private sheets still need the Sheets API + opt-in credentials. Keep the same data as a local snapshot fixture so fixture-mode and live-mode assert identical rows."
    status: pending
  - id: connections-federated
    content: "Cloud-resource connections + federated query paths (EXTERNAL_QUERY, BigLake, object tables, Spanner external datasets). Wire the bqconnection gRPC + REST surface to the config model so a connection resolves to fixture/local data or an opt-in live federated endpoint. Scope: read paths first; document which connection types stay unsupported."
    status: pending
  - id: table-definitions
    content: "Extend ephemeral tableDefinitions on jobs.query / jobs.insert to non-GCS sources beyond today's GCS-via-fake-gcs materialization, routing each definition through the config model (gateway/external/materialize.go)."
    status: pending
  - id: fixtures-trackers
    content: "Conformance fixtures: gs:// LOAD/EXPORT round-trip against fake-gcs, a Google Sheets external-table read from a local snapshot, and a connection/federated read in fixture mode. The Sheets fixture snapshots the public sample sheet (docId 1BxiMVs0XRA5nFMdKvBdBZjgmUUqptlbs74OgvE2upms; columns Student Name, Gender, Class Level, Home State, Major, Extracurricular Activity) so the default offline run is hermetic and the opt-in live test (live-sheets-test) asserts the same rows. Update ENGINE_POLICY (gs:// LOAD/EXPORT, Google Sheets, connections rows), the §Google Sheets section, ROADMAP §External data sources (⏳ -> ✅ per surface). Document the live-mode opt-in + credentials in docs/REST_API.md + README."
    status: pending
  - id: live-sheets-test
    content: "Opt-in live-upstream test using the public Google sample sheet (docId 1BxiMVs0XRA5nFMdKvBdBZjgmUUqptlbs74OgvE2upms, tab \"Class Data\"): define an external table in GOOGLE_SHEETS mode pointing at the sheet, configure that source as `live` via the config model, and assert a SELECT returns the 30 student rows that match the committed local snapshot fixture. The test MUST be gated behind an explicit opt-in (e.g. BIGQUERY_EMULATOR_LIVE_SHEETS=1 / a build tag) and network-skip cleanly when unset, so the default conformance + CI runs stay offline and deterministic. Use the unauthenticated CSV export for the link-public read path; document that private sheets require Sheets-API credentials."
    status: pending
  - id: skip-audit
    content: "Third-party + conformance skip audit (run before declaring done; never just remove the obvious rows). Re-run each suite the landed surface should unblock and unskip what now passes: python `test_query_external_sheets_permanent_table` / `test_query_external_sheets_temporary_table` + GCS-backed sample loads (third_party/python-bigquery-tests/emulator_pytest_skip.py); golang bqconnection / external-table ITs (third_party/golang-bigquery-tests/bqtestutil/emulator_skip.go + the README coverage matrix); java bigqueryconnection ITs (third_party/java-bigquery-tests, currently UNIMPLEMENTED per README); GCS subtests gated on STORAGE_EMULATOR_HOST. Update third_party/README.md skip matrices + node-bigquery-tests/EMULATOR.md. Leave a written note for any row that still can't run + why."
    status: pending
---

# Expand 01 — External data sources

## Why

[ROADMAP.md §External data sources](../../ROADMAP.md) and the user want
external sources configurable as **either** deterministic fixtures /
locally-stored / GCS data **or** opt-in **live** upstreams (a real
Google Sheets document, a real GCS bucket, a federated endpoint). Today:

- Local `file://` LOAD/EXPORT works (`RunLoadData`, EXPORT `COPY`).
- GCS-backed external tables work when `STORAGE_EMULATOR_HOST` points at
  fake-gcs (`gateway/external/materialize.go`).
- `gs://` LOAD/EXPORT DDL is `unsupported`; Google Sheets external
  tables return **501**; connection-backed federated scans are
  metadata-only (`third_party/README.md`, ENGINE_POLICY §Google Sheets).

## The hard part

The project's standing non-goal is "no cloud passthrough for query
execution." This plan threads that needle: **query execution still runs
locally**; only the *data fetch/write for a configured external source*
may reach a live upstream, and only when the user opts in per source.
The config model is the load-bearing piece — it must make fixture/local
the default and make `live` explicit, credentialed, and per-source.

## Key files

- [`gateway/external/materialize.go`](../../gateway/external/materialize.go) — external-table materialization (extend with the config model)
- [`gateway/handlers/external_query.go`](../../gateway/handlers/external_query.go) — `tableDefinitions` + `EXTERNAL_QUERY` resolution
- [`gateway/handlers/external_tables_test.go`](../../gateway/handlers/external_tables_test.go) — existing external-table coverage
- [`gateway/handlers/metadata_store.go`](../../gateway/handlers/metadata_store.go) — `externalDataConfiguration` round-trip
- [`gateway/handlers/bqconnection/`](../../gateway/handlers/bqconnection/) — connection gRPC/REST surface
- `RunLoadData` / EXPORT `COPY` control-op paths (engine side) — gs:// branch
- [`docs/REST_API.md`](../../docs/REST_API.md), [`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md) §Google Sheets, [`third_party/README.md`](../../third_party/README.md) — docs + skip matrices

## Steps

1. Config model: per-source `fixture | local | live` resolution + default-to-local.
2. gs:// LOAD/EXPORT through the model (fake-gcs / local snapshot / live bucket).
3. Google Sheets external tables (local snapshot + opt-in live fetch); drop the 501.
4. Connections + federated read paths through the model.
5. Ephemeral `tableDefinitions` for non-GCS sources.
6. Fixtures + tracker/posture flips + skip-matrix removal + live-mode docs.

## Live-mode integration test (public sample sheet)

The opt-in `live` path is proven against the public Google sample sheet
([Example Spreadsheet](https://docs.google.com/spreadsheets/d/1BxiMVs0XRA5nFMdKvBdBZjgmUUqptlbs74OgvE2upms/edit),
docId `1BxiMVs0XRA5nFMdKvBdBZjgmUUqptlbs74OgvE2upms`, tab **Class Data**):

| Column | Type |
|--|--|
| `Student Name` | STRING |
| `Gender` | STRING |
| `Class Level` | STRING |
| `Home State` | STRING |
| `Major` | STRING |
| `Extracurricular Activity` | STRING |

30 student rows (`Alexandra` … `Will`).

- **Why this sheet:** it is **link-public**, so the live read path can use
  the unauthenticated CSV export
  (`https://docs.google.com/spreadsheets/d/1BxiMVs0XRA5nFMdKvBdBZjgmUUqptlbs74OgvE2upms/export?format=csv&gid=<gid>`)
  with **no credentials** — a real upstream fetch that still works in CI
  without secrets. Private sheets remain Sheets-API + credential gated.
- **Hermetic by default:** commit the same 30 rows as a local snapshot
  fixture (fixture-mode reads it). The live test reuses that fixture as
  its golden assertion, so fixture-mode and live-mode prove identical rows.
- **Opt-in only:** gate the live fetch behind an explicit flag/env
  (e.g. `BIGQUERY_EMULATOR_LIVE_SHEETS=1`) and **network-skip** when unset
  so `task conformance:run` and CI stay offline and deterministic.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run                       # offline; Sheets read uses the local snapshot
task lint:dispositions
task thirdparty:python                      # external sheets samples
BIGQUERY_EMULATOR_LIVE_SHEETS=1 task conformance:run   # opt-in: real fetch of the public sample sheet
task bazel:shutdown && task bazel:status
```

## Third-party / conformance to revisit

When a surface lands, **audit the skip matrices** — some currently-skipped
tests should run afterward. Re-run the suite to prove it before removing a
skip; leave a note for anything still blocked.

- **python** — `test_query_external_sheets_*` and GCS-backed sample loads
  (`third_party/python-bigquery-tests/emulator_pytest_skip.py`).
- **golang** — connection / external-table ITs gated in
  `third_party/golang-bigquery-tests/bqtestutil/emulator_skip.go` + the
  README "coverage matrix".
- **java** — `bigqueryconnection` ITs (today `UNIMPLEMENTED`).
- **all lanes** — GCS subtests gated on `STORAGE_EMULATOR_HOST`; update
  `third_party/README.md` + `node-bigquery-tests/EMULATOR.md`.

## Out of scope

- BigQuery Omni (cross-cloud AWS/Azure regions) — stays a non-goal.
- SQL pushdown optimization into federated sources (semantics, not perf).
- Write-back to live Google Sheets beyond what BigQuery itself supports.
- Connection IAM / permission enforcement (metadata-only, as today).
