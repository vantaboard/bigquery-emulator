# BigQuery Emulator UI — HTTP API contract

The React UI calls the **BigQuery REST API** (`/bigquery/v2/*`) exposed by [`bigquery-emulator`](https://github.com/vantaboard/bigquery-emulator) (or real BigQuery). This UI repo does not embed Go.

Base URL is configured via `VITE_API_URL` (empty string means same origin in production; in dev, Vite proxies `/bigquery` and `/api/emulator` to **`VITE_PROXY_TARGET`**, default `http://127.0.0.1:9050`).

## BigQuery REST endpoints used

| UI action | BigQuery REST |
|-----------|---------------|
| List projects | `GET /bigquery/v2/projects` |
| List datasets | `GET /bigquery/v2/projects/{projectId}/datasets` |
| List tables | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables` |
| Table metadata | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}` |
| Run query | `POST /bigquery/v2/projects/{projectId}/queries` with `{ "query": "...", "useLegacySql": false }` |

The client transforms BigQuery JSON (e.g. `projects[].id`, `schema.fields`, `rows[].f[].v`) into the shapes consumed by React components.

## SQL Tools API (emulator only, M4+)

Not part of the public BigQuery REST surface. Enabled on the emulator gateway with `--enable-sql-tools-api`. See [SQL_TOOLS_API.md](https://github.com/vantaboard/bigquery-emulator/blob/main/docs/SQL_TOOLS_API.md) in the emulator repo.

| UI action | Emulator route |
|-----------|----------------|
| Format SQL | `POST /api/emulator/sql/format` |
| Parse / syntax diagnostics | `POST /api/emulator/sql/parse` |
| Catalog-aware completion | `POST /api/emulator/sql/complete` |

The UI probes for SQL Tools at session start and falls back to client-side `sql-formatter` and `@codemirror/lang-sql` when unavailable (e.g. real BigQuery or an older emulator image).

**Access:** loopback callers only by default. For Docker Compose or LAN, the emulator needs `--sql-tools-api-allow-remote` (and optionally `--sql-tools-api-token`). When a token is configured, send header `X-BigQuery-Emulator-SqlTools-Token` (dev: set `VITE_SQL_TOOLS_TOKEN`).

**Offsets:** `/complete` uses UTF-8 byte offsets; the CodeMirror integration converts to editor positions.

## Optional UI config

| Variable | Purpose |
|----------|---------|
| `VITE_DEFAULT_PROJECT` | When set, the UI also lists this project if it has datasets but is missing from `GET /bigquery/v2/projects` (common with seeded emulator fixtures) |
| `VITE_ALLOW_EMULATOR_PROJECT_ADMIN` | `true` to show emulator project admin UI (creation still requires a compatible backend endpoint) |
| `VITE_SQL_TOOLS_TOKEN` | Optional token for SQL Tools API when the emulator requires `--sql-tools-api-token` |

## Notes

- BigQuery errors use the envelope: `{ "error": { "message": "...", "code": 404, ... } }`.
- SQL Tools errors use: `{ "code": number, "status": "invalid", "message": "..." }`.
- CORS: the emulator should allow the UI origin in development (Vite) and your production host when not same-origin.
