---
name: Load jobs and local ingest
overview: Load jobs and multipart upload work for gs/file paths; document limits and defer cross-cloud schemes to plan 11.
todos:
  - id: document-load-matrix
    content: Enumerate supported sourceFormat and URI schemes in docs/REST_API.md
    status: pending
  - id: e2e-multipart-csv
    content: e2e multipart CSV upload → tabledata.list reads rows
    status: pending
  - id: storage-emulator-note
    content: Document STORAGE_EMULATOR_HOST requirement for gs:// in dev
    status: pending
isProject: false
---

# 05 — Load jobs + local file ingest + multipart upload

- **UI gap:** #5 (priority **P5**)
- **Verified state at HEAD (`60d19b3e`):** **Partial** — local/file + gs submit OK; `s3://` unsupported.
- **Related:** cross-cloud external tables → [`11-external-source-ingestion.plan.md`](11-external-source-ingestion.plan.md)

## Current state at HEAD (grounded)

Dispatch: `gateway/handlers/jobs.go` (~159–160) `cfg.Load` → `runSyncLoadInsert`
→ `gateway/load/executor.go` (~26–98). Fetch: `gateway/load/fetch.go` (~16–31).
Multipart/resumable: `gateway/handlers/jobs_upload.go` (~179–269) +
`gateway/load/upload.go`. Tests: `gateway/handlers/jobs_load_test.go`.

| Source scheme | State | Anchor |
|---------------|-------|--------|
| `gs://` | ✅ via `STORAGE_EMULATOR_HOST` / `FAKE_GCS_PORT` | `fetch.go` ~67–88 |
| `file://` | ✅ | `fetch.go` ~23–25; test ~17–51 |
| absolute path | ✅ | `fetch.go` ~27–28 |
| `s3://` | ❌ `unsupported sourceUri scheme` | `fetch.go` ~30 |
| `https://` (direct) | ❌ in load path | — |

| Format | State |
|--------|-------|
| CSV (autodetect, skipLeadingRows, delimiter, quote) | ✅ |
| NEWLINE_DELIMITED_JSON | ✅ |
| PARQUET | ✅ |
| AVRO | ✅ (test ~174+) |
| ORC | ✅ |
| DATASTORE_BACKUP | ✅ (URI-based) |
| GOOGLE_SHEETS | ❌ in load path (external-table concern) |

| Upload mode | State |
|-------------|-------|
| `?uploadType=multipart` | ✅ `jobs_upload.go` ~179–207 |
| resumable (init + PUT chunks) | ✅ `jobs_upload.go` ~209–269; test ~81–159 |

## Goal / done-criteria (UI-observable)

1. Document precisely which source schemes/formats work in emulator mode so the
   UI can present accurate errors/fallbacks (UI brief explicitly asks for this).
2. Local/file-backed dev ingest works end-to-end (already true — lock with a
   doc + e2e).
3. Multipart upload from the UI's Create Table flow works end-to-end (already
   true — verify against the UI's exact request shape).
4. (Optional) `s3://` and/or `https://` direct fetch if the UI/users need them.

## Implementation steps

### Step 1 — Document supported load surface

Add/extend a docs section (e.g. `docs/REST_API.md` load rows + a short
`docs/guides` note) enumerating: supported `sourceFormat` values, supported URI
schemes (`gs://` requires storage emulator config; `file://` / absolute for dev),
multipart + resumable upload, and the unsupported set (`s3://`, `https://`,
`GOOGLE_SHEETS`). The UI surfaces these as expected errors.

### Step 2 — Verify the UI's multipart request shape

Confirm `POST /upload/bigquery/v2/projects/{p}/jobs?uploadType=multipart` with a
`job` JSON part + file part matches the UI exactly; add an e2e that posts a small
CSV via multipart and reads it back via `tabledata.list`.

### Step 3 — (Optional) add `s3://` / `https://` fetch

In `gateway/load/fetch.go` `FetchSource`, add an `s3://` branch (config via env,
mirroring the `gs://` storage-emulator pattern) and/or a generic `https://`
fetch. Only do this if there is real demand — emulator dev workflows are covered
by `file://` / `gs://`.

### Step 4 — `STORAGE_EMULATOR_HOST` ergonomics note

Document that `gs://` defaults to the storage emulator unless
`STORAGE_EMULATOR_HOST` points elsewhere, and how to wire a real bucket for dev.

## Tests

- `gateway/handlers/jobs_load_test.go`: already covers `file://` CSV, AVRO,
  WRITE_APPEND, multipart, resumable. Add a multipart-from-UI-shape case (Step 2).
- `gateway/e2e`: multipart upload → `tabledata.list` round-trip.
- If Step 3 lands: a `s3://` fetch test behind an env-gated fake.

## Out of scope

- Real cloud credentials / live S3/GCS/Azure/Drive/Bigtable.
- `GOOGLE_SHEETS` (external-table path, not load).

## Touch list

`docs/REST_API.md` + `docs/guides`, `gateway/handlers/jobs_load_test.go`,
`gateway/e2e/`, optionally `gateway/load/fetch.go`.
