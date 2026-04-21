# BigQuery Emulator UI

React + TypeScript web UI for exploring BigQuery (emulator or real), modeled after a slim [Vite](https://vitejs.dev/) + [TanStack Query](https://tanstack.com/query) setup similar to internal Prospero web apps.

There is **no Go code in this repository**. The JSON API is served by [`bigquery-emulator`](https://github.com/vantaboard/bigquery-emulator) under **`/api/*`** on the **same port** as the BigQuery REST API (default **9050**).

## Features

- Browse projects, datasets, and tables (multi-project when the emulator supports discovery)
- View table metadata and schema
- Run SQL, format with BigQuery dialect, view tabular and JSON results
- Shareable URLs (query encoded in the query string)
- Optional emulator project admin when the API enables it

See [docs/api-contract.md](./docs/api-contract.md) and [docs/rollout-checklist.md](./docs/rollout-checklist.md).

## Prerequisites

- **Node.js 22+** and npm
- A running **bigquery-emulator** (or real BigQuery with credentials) exposing **`/api`** on the HTTP port you configure

## Run the emulator (API + BigQuery)

From a checkout of **`bigquery-emulator`** that builds on your machine:

```bash
./bigquery-emulator --port 9050
# or: go run ./cmd/bigquery-emulator --port 9050
```

The explorer JSON API is available at **`http://127.0.0.1:9050/api/...`** alongside **`/bigquery/v2/...`**.

## Run the UI (development)

```bash
cd bigquery-emulator-ui
npm install
cp .env.example .env   # optional: tune VITE_PROXY_TARGET
npm run dev
```

[Vite](https://vitejs.dev/) serves the app (default **5173**) and proxies **`/api`** to **`VITE_PROXY_TARGET`** (default `http://127.0.0.1:9050`).

## Production build

```bash
npm run build
npm run preview
```

For a static build that talks to the API on another origin, set **`VITE_API_URL`** at build time (e.g. `https://api.example.com`).

## Docker

`docker compose up` runs the **goccy** emulator as **`bigquery`** and the static UI (**`bq-ui`**) on **8080**. Nginx proxies **`/api`** to **`http://bigquery:9050`** (same container as the merged explorer API).

For the Vantaboard emulator image, use **`docker-compose.local.yaml`** to override the `bigquery` service.

## Environment (explorer API in bigquery-emulator)

| Variable | Purpose |
|----------|---------|
| `BIGQUERY_EMULATOR_HOST` | If unset, the emulator sets loopback + `--port` for the embedded explorer client; override to aim at another endpoint |
| `BIGQUERY_PROJECT_IDS` / `BIGQUERY_PROJECT_IDS_MODE` | Optional project list merge/override |
| `ALLOW_EMULATOR_PROJECT_ADMIN` | `true` to allow `POST /api/emulator/projects` |
| `PORT` | Only used by the **standalone** explorer binary (removed); merged API uses **`--port`** on the main emulator |

## Development scripts

| Script | Description |
|--------|-------------|
| `npm run dev` | Vite dev server |
| `npm run build` | Typecheck + production bundle |
| `npm run preview` | Preview production build |
| `npm run test` | Vitest |
| `npm run lint` | ESLint |

## CI

GitHub Actions builds and pushes the **nginx + static UI** Docker image (see `.github/workflows/docker-build.yml`).

## License

See the repository license file.
