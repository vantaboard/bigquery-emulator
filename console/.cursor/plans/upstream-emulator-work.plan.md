---
name: Upstream emulator work
overview: Track BigQuery REST behaviors missing or incomplete in vantaboard/bigquery-emulator. UI builds to the real contract; gaps degrade gracefully.
todos: []
isProject: false
---

# Upstream emulator work

Parallel track for filing issues/PRs against [vantaboard/bigquery-emulator](https://github.com/vantaboard/bigquery-emulator).

## Known gaps (M1-01 API layer)

| Endpoint / behavior | BigQuery REST path | Notes |
|---------------------|-------------------|-------|
| Dataset metadata | `GET .../datasets/{datasetId}` | May return partial metadata vs full BigQuery shape |
| Routines list/detail | `GET .../datasets/{datasetId}/routines` | Routines support may be missing |
| Table data preview | `GET .../tables/{tableId}/data` | `tabledata.list` may be unsupported; fallback is `SELECT * LIMIT n` query |
| Schema patch | `PATCH .../tables/{tableId}` | Table schema updates may not be implemented |
| Delete table/dataset | `DELETE .../tables/{tableId}`, `DELETE .../datasets/{datasetId}` | Delete operations may be missing |
| Jobs (copy/snapshot/load) | `POST .../jobs`, `GET .../jobs/{jobId}` | Async copy/snapshot/load jobs may be unsupported |

When implementing UI for these flows, verify against a running emulator and add concrete reproduction steps here.
