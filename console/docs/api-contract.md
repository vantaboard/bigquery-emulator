# BigQuery Emulator UI — HTTP API contract

The React UI talks to the **`/api/*` JSON routes** served by [`bigquery-emulator`](https://github.com/vantaboard/bigquery-emulator) on the **same HTTP port** as the BigQuery REST API (see `internal/explorerapi`). This UI repo does not embed Go.

Base URL is configured via `VITE_API_URL` (empty string means same origin in production; in dev, Vite proxies `/api` to **`VITE_PROXY_TARGET`**, default `http://127.0.0.1:9050`).

## Endpoints

### `GET /api/config`

**Response** (`200`)

```json
{ "allowEmulatorProjectAdmin": true }
```

`allowEmulatorProjectAdmin` is `true` only when the server enables admin and an emulator host is configured.

---

### `GET /api/projects`

**Response** (`200`): JSON array of project id strings.

```json
["my-project", "other-project"]
```

---

### `POST /api/emulator/projects`

Create a project on a compatible emulator (when admin is allowed).

**Request body**

```json
{ "id": "new-project-id" }
```

**Responses**

- `201`: `{ "id": "new-project-id" }`
- `403` / `400` / `500`: `{ "error": "message" }`

---

### `GET /api/projects/:project_id/datasets`

**Response** (`200`): JSON array of dataset id strings.

---

### `GET /api/projects/:project_id/datasets/:dataset_id/tables`

**Response** (`200`): JSON array of table id strings.

---

### `GET /api/projects/:project_id/datasets/:dataset_id/tables/:table_id/schema`

**Response** (`200`): Table metadata including schema.

```json
{
  "schema": [
    {
      "name": "col",
      "type": "STRING",
      "mode": "NULLABLE",
      "description": null
    }
  ],
  "numRows": 0,
  "numBytes": 0,
  "creationTime": "2020-01-01T00:00:00Z",
  "lastModified": "2020-01-01T00:00:00Z",
  "description": "",
  "type": "TABLE",
  "location": "",
  "fullyQualifiedName": "proj.dataset.table"
}
```

---

### `POST /api/query`

**Request body**

```json
{ "query": "SELECT 1" }
```

**Response** (`200`)

```json
{
  "columns": ["f0_"],
  "rows": [{ "f0_": 1 }],
  "total_rows": 1
}
```

**Error** (`4xx`/`5xx`): `{ "error": "message" }`

## Rewrite notes

- Shapes above reflect the legacy explorer API; the backend may evolve. Prefer versioning (`/api/v1/...`) if breaking changes are introduced later.
- CORS: the explorer API should allow the UI origin in development (Vite) and your production host.
