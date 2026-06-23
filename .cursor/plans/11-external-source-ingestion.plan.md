---
name: External source ingestion
overview: Support external-table and load-job URIs for Bigtable, Azure, Drive, and optional s3/https so Create Table source pickers work beyond local file/gs dev paths.
todos:
  - id: inventory-schemes
    content: Document current scheme matrix in docs/REST_API.md (gs/file OK; s3/https/azure/drive/bigtable gaps)
    status: pending
  - id: bigtable-external-uri
    content: Accept Bigtable external URIs (https://googleapis.com/bigtable/...) in tables.insert externalDataConfiguration
    status: pending
  - id: azure-drive-stubs
    content: Add Azure Blob and Google Drive URI handlers or explicit BQ-shaped errors if full fetch is out of scope
    status: pending
  - id: s3-load-fetch
    content: Optional s3:// branch in gateway/load/fetch.go mirroring gs:// emulator pattern
    status: pending
  - id: e2e-external-smoke
    content: e2e for Bigtable external table definition insert + load-job error shapes for unsupported schemes
    status: pending
isProject: false
---

# 11 — External source ingestion (GCS / S3 / Azure / Drive / Bigtable)

- **UI gap:** #13
- **Priority:** **P5** (Create Table external-source modals)
- **Verified state at HEAD (`60d19b3e`):** **Open**
- **Related:** plan 05 covers **load jobs** for `gs://`/`file://`; this plan covers
  **external tables** and cross-cloud schemes the UI submits.

## Symptom (UI brief repro)

**Bigtable external table** via `tables.insert`:

```json
{
  "externalDataConfiguration": {
    "sourceFormat": "BIGTABLE",
    "sourceUris": ["https://googleapis.com/bigtable/projects/p/instances/i/tables/t"]
  }
}
```

→ 400: `unsupported sourceUri scheme: "https://googleapis.com/bigtable/..."`

**Load job `s3://`:**

→ `unsupported sourceUri scheme: "s3://bucket/x.csv"` (`gateway/load/fetch.go` ~30)

**Load job `gs://`** with no storage emulator:

→ job `DONE` with `errorResult` fetch failure (expected in dev).

## Current state at HEAD (grounded)

| Path | Scheme / format | State | Anchor |
|------|-----------------|-------|--------|
| Load job | `gs://` | ✅ submit; fetch via storage emulator | `gateway/load/fetch.go` ~67–88 |
| Load job | `file://` / abs path | ✅ | `fetch.go` ~23–28 |
| Load job | `s3://` | ❌ | `fetch.go` ~30 |
| Load job | `https://` direct | ❌ | — |
| External table | `gs://` CSV/Parquet/etc. | ⚠️ partial | engine external table path |
| External table | Bigtable `https://…bigtable…` | ❌ | external table URI validator |
| External table | Azure / Drive | ❌ | not wired |
| Multipart upload | UI Create Table | ✅ | `jobs_upload.go` ~179–269 (plan 05) |

## Goal / done-criteria (UI-observable)

1. **Bigtable:** `POST .../tables` with Bigtable `externalDataConfiguration` returns
   200 and the table appears in `tables.list` with `type: EXTERNAL` (query may
   return stub/empty or a documented limitation — match BQ error shape if execution
   is not implemented).
2. **Azure / Drive:** either fetch-backed support for dev URIs **or** a clear
   BigQuery-consistent 400 with reason the UI can display (no generic 501).
3. **`s3://` load jobs:** optional env-configured fetch (mirror `STORAGE_EMULATOR_HOST`
   pattern) **or** documented unsupported with stable error message.
4. Docs enumerate which sources work in emulator mode so the UI can set expectations.

## Implementation steps

### Step 1 — Bigtable URI normalization

In the gateway external-table insert path (`gateway/handlers/tables_insert.go` and
engine external config builder):

- Recognize `https://googleapis.com/bigtable/v2/...` and legacy Bigtable URI forms.
- Map to an internal Bigtable external config the engine can register (even if reads
  return empty or “not connected” until a mock Bigtable backend exists).

### Step 2 — Azure Blob + Google Drive

Survey UI request shapes from bigquery-emulator-ui Create Table flow. Add URI scheme
handlers (`azure://`, `https://drive.google.com/...`) or explicit validation errors
matching BigQuery `invalid` reason codes.

### Step 3 — `s3://` load fetch (optional)

Extend `gateway/load/fetch.go` with an `s3://` branch:

- Env vars for endpoint/credentials (dev-only), similar to GCS emulator wiring.
- Defer if no demand — but document in `docs/REST_API.md`.

### Step 4 — Docs + e2e

- Update `docs/REST_API.md` external table + load URI matrix.
- `gateway/e2e/external_sources_test.go` — Bigtable insert 200; s3 load error shape.

## Tests

- Handler unit tests for URI parsing (Bigtable https form).
- e2e: external table insert + list; load job unsupported scheme message stable.

## Out of scope

- Full live Bigtable/Azure/Drive data plane (production credentials, paging reads).
- BigQuery Storage Read API for external bytes.

## Touch list

`gateway/handlers/tables_insert.go`, `gateway/load/fetch.go`,
`backend/engine/...` external table registration, `docs/REST_API.md`,
`gateway/e2e/external_sources_test.go`.
