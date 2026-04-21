# BigQuery Emulator UI

React + TypeScript web UI for exploring BigQuery (emulator or real), modeled after a slim [Vite](https://vitejs.dev/) + [TanStack Query](https://tanstack.com/query) setup similar to internal Prospero web apps.

There is **no Go code in this repository**. The HTTP API lives in the [`bigquery-emulator`](https://github.com/vantaboard/bigquery-emulator) repo as **`cmd/bq-explorer-api`**.

## Features

- Browse projects, datasets, and tables (multi-project when the emulator supports discovery)
- View table metadata and schema
- Run SQL, format with BigQuery dialect, view tabular and JSON results
- Shareable URLs (query encoded in the query string)
- Optional emulator project admin when the API enables it

See [docs/api-contract.md](./docs/api-contract.md) and [docs/rollout-checklist.md](./docs/rollout-checklist.md).

## Prerequisites

- **Node.js 22+** and npm
- A running **BigQuery emulator** or Google BigQuery credentials for real data
- The **explorer API** binary from `bigquery-emulator` (see below)

## Run the explorer API

From a checkout of **`bigquery-emulator`** that builds cleanly on your machine (same `go.mod` / sibling libs as that project):

```bash
cd /path/to/bigquery-emulator
export BIGQUERY_EMULATOR_HOST=localhost:9050
# optional: ALLOW_EMULATOR_PROJECT_ADMIN=true
go run ./cmd/bq-explorer-api
```

Default API port is **8000**.

## Run the UI (development)

```bash
cd bigquery-emulator-ui
npm install
cp .env.example .env   # optional: tune VITE_PROXY_TARGET
npm run dev
```

[Vite](https://vitejs.dev/) serves the app (default **5173**) and proxies **`/api`** to **`VITE_PROXY_TARGET`** (default `http://127.0.0.1:8000`).

## Production build

```bash
npm run build
npm run preview
```

For a static build that talks to the API on another origin, set **`VITE_API_URL`** at build time (e.g. `https://api.example.com`).

## Docker

`docker compose up` starts the **goccy** emulator and an **nginx** container on **8080** that proxies **`/api`** to **`host.docker.internal:8000`**. Run **`bq-explorer-api`** on the **host** on port **8000** so the UI can reach it.

For the Vantaboard emulator image, keep using **`docker-compose.local.yaml`** to override the `bigquery` service, then start the API on the host as above with `BIGQUERY_EMULATOR_HOST=localhost:9050`.

## Environment (API)

The explorer process uses the same variables as the legacy UI server, including:

| Variable | Purpose |
|----------|---------|
| `BIGQUERY_EMULATOR_HOST` | e.g. `localhost:9050` (no scheme) |
| `BIGQUERY_PROJECT_IDS` / `BIGQUERY_PROJECT_IDS_MODE` | Optional project list merge/override |
| `ALLOW_EMULATOR_PROJECT_ADMIN` | `true` to allow `POST /api/emulator/projects` |
| `PORT` | API listen port (default `8000`) |

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
