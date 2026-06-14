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
    content: "Google Sheets external tables (GOOGLE_SHEETS / googleSheetsOptions): replace the gateway 501 with (fixture/local) a locally-stored sheet snapshot (CSV/JSON under --data_dir or a conformance fixture) materialized into the engine catalog at insert time, and (live) an opt-in Google Sheets API fetch. Round-trip externalDataConfiguration through MetadataStore as today."
    status: pending
  - id: connections-federated
    content: "Cloud-resource connections + federated query paths (EXTERNAL_QUERY, BigLake, object tables, Spanner external datasets). Wire the bqconnection gRPC + REST surface to the config model so a connection resolves to fixture/local data or an opt-in live federated endpoint. Scope: read paths first; document which connection types stay unsupported."
    status: pending
  - id: table-definitions
    content: "Extend ephemeral tableDefinitions on jobs.query / jobs.insert to non-GCS sources beyond today's GCS-via-fake-gcs materialization, routing each definition through the config model (gateway/external/materialize.go)."
    status: pending
  - id: fixtures-trackers
    content: "Conformance fixtures: gs:// LOAD/EXPORT round-trip against fake-gcs, a Google Sheets external-table read from a local snapshot, and a connection/federated read in fixture mode. Update ENGINE_POLICY (gs:// LOAD/EXPORT, Google Sheets, connections rows), the §Google Sheets section, ROADMAP §External data sources (⏳ -> ✅ per surface), and remove the python `test_query_external_sheets_*` skip rows that now pass. Document the live-mode opt-in + credentials in docs/REST_API.md + README."
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

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task thirdparty:python      # external sheets samples
task bazel:shutdown && task bazel:status
```

## Out of scope

- BigQuery Omni (cross-cloud AWS/Azure regions) — stays a non-goal.
- SQL pushdown optimization into federated sources (semantics, not perf).
- Write-back to live Google Sheets beyond what BigQuery itself supports.
- Connection IAM / permission enforcement (metadata-only, as today).
