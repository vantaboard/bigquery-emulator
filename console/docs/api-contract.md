# BigQuery Emulator UI — HTTP API contract

The React UI calls the **BigQuery REST API** (`/bigquery/v2/*`) exposed by [`bigquery-emulator`](https://github.com/vantaboard/bigquery-emulator) (or real BigQuery). This UI repo does not embed Go.

Base URL is configured via `VITE_API_URL` (empty string means same origin in production; in dev, Vite proxies `/bigquery` to **`VITE_PROXY_TARGET`**, default `http://127.0.0.1:9050`).

## Endpoints used

| UI action | BigQuery REST |
|-----------|---------------|
| List projects | `GET /bigquery/v2/projects` |
| List datasets | `GET /bigquery/v2/projects/{projectId}/datasets` |
| List tables | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables` |
| Table metadata | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}` |
| Run query | `POST /bigquery/v2/projects/{projectId}/queries` with `{ "query": "...", "useLegacySql": false }` |

The client transforms BigQuery JSON (e.g. `projects[].id`, `schema.fields`, `rows[].f[].v`) into the shapes consumed by React components.

## Optional UI config

| Variable | Purpose |
|----------|---------|
| `VITE_DEFAULT_PROJECT` | When set, the UI also lists this project if it has datasets but is missing from `GET /bigquery/v2/projects` (common with seeded emulator fixtures) |
| `VITE_ALLOW_EMULATOR_PROJECT_ADMIN` | `true` to show emulator project admin UI (creation still requires a compatible backend endpoint) |

## Notes

- Errors use the BigQuery envelope: `{ "error": { "message": "...", "code": 404, ... } }`.
- CORS: the emulator should allow the UI origin in development (Vite) and your production host when not same-origin.
