# Load jobs (local & emulator ingest)

The BigQuery UI **Create table from upload** and **Load from GCS** flows
use `jobs.insert` with `configuration.load`. The emulator runs these jobs
synchronously and returns `state: DONE` when ingest completes.

## Supported formats

- `CSV` — with `skipLeadingRows`, `fieldDelimiter`, `quote`, `autodetect`
- `NEWLINE_DELIMITED_JSON`
- `AVRO`, `PARQUET`, `ORC`
- `DATASTORE_BACKUP` (URI-based)

## Source URIs

| Scheme | When to use |
|--------|-------------|
| `file:///path/to/data.csv` | Local development; no cloud credentials |
| `/absolute/path/to/data.csv` | Same as `file://` |
| `gs://bucket/object` | Requires the bundled fake-gcs server (`FAKE_GCS_PORT`) or `STORAGE_EMULATOR_HOST` pointing at a storage emulator |

**Not supported:** `s3://`, direct `https://` URLs, and `GOOGLE_SHEETS`
(use `externalDataConfiguration` on `tables.insert` for sheets-backed
external tables).

## Multipart upload (UI shape)

The UI posts to:

`POST /upload/bigquery/v2/projects/{projectId}/jobs?uploadType=multipart`

with a `multipart/related` body: a JSON `job` part describing
`configuration.load.destinationTable` + format options, plus a file part
containing the bytes to ingest. The handler mirrors
`TestJobInsertUploadMultipartCSV` in `gateway/handlers/jobs_load_test.go`.

## Resumable upload

1. `POST ...?uploadType=resumable` with the job JSON and
   `X-Upload-Content-Length`.
2. `PUT` to the `Location` header URL with the file bytes.

## GCS emulator wiring

Set `STORAGE_EMULATOR_HOST=http://127.0.0.1:<fake-gcs-port>` (or rely on
the default fake-gcs port configured at gateway startup) so `gs://` URIs
resolve without real Google Cloud credentials.
