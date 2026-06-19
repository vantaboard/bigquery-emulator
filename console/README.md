# BigQuery Emulator UI

React + TypeScript web UI for exploring BigQuery (emulator or real), modeled after a slim [Vite](https://vitejs.dev/) + [TanStack Query](https://tanstack.com/query) setup similar to internal Prospero web apps.

There is **no Go code in this repository**. The UI talks to the **BigQuery REST API** (`/bigquery/v2/*`) served by [`bigquery-emulator`](https://github.com/vantaboard/bigquery-emulator) on the default HTTP port **9050**.

## Features

- Browse projects, datasets, and tables (multi-project when the emulator supports discovery)
- View table metadata and schema
- Run SQL, format with BigQuery dialect, view tabular and JSON results
- Shareable URLs (query encoded in the query string)

See [docs/api-contract.md](./docs/api-contract.md) and [docs/rollout-checklist.md](./docs/rollout-checklist.md).

## Prerequisites

- **Node.js 22+** and npm
- A running **bigquery-emulator** (or real BigQuery with credentials) exposing **`/bigquery/v2`** on the HTTP port you configure

## Run the emulator

```bash
docker run --rm -p 9050:9050 ghcr.io/vantaboard/bigquery-emulator:v0.3.1
# or from a local checkout: task emulator:run-full
```

## Run the UI (development)

```bash
cd bigquery-emulator-ui
npm install
cp .env.example .env   # optional: tune VITE_PROXY_TARGET, VITE_DEFAULT_PROJECT
npm run dev
```

[Vite](https://vitejs.dev/) serves the app (default **5173**) and proxies **`/bigquery`** to **`VITE_PROXY_TARGET`** (default `http://127.0.0.1:9050`).

## Production build

```bash
npm run build
npm run preview
```

For a static build that talks to the API on another origin, set **`VITE_API_URL`** at build time (e.g. `https://api.example.com`).

## Docker

`docker compose up` runs the **goccy** emulator as **`bigquery`** and the static UI (**`bq-ui`**) on **8080**. Nginx proxies **`/bigquery`** to **`http://bigquery:9050`**.

For the Vantaboard emulator image, use **`docker-compose.local.yaml`** to override the `bigquery` service.

## E2E testing

End-to-end tests use a dedicated Compose stack with the pinned Vantaboard emulator **`ghcr.io/vantaboard/bigquery-emulator:v0.3.1`**:

```bash
npm install
npm run test:e2e          # starts stack, runs Playwright, tears down
npm run test:e2e:ui       # Playwright UI mode (stack must be up)
npm run e2e:up            # start stack only (UI on 8080)
npm run e2e:down          # stop stack and remove volumes
```

The E2E stack is defined in **`docker-compose.e2e.yaml`**. Seed data lives in **`e2e/fixtures/seed.yaml`**. CI runs the same suite via **`.github/workflows/e2e.yml`**.

## Environment

| Variable | Purpose |
|----------|---------|
| `VITE_PROXY_TARGET` | Dev: Vite proxy target for `/bigquery` (default `http://127.0.0.1:9050`) |
| `VITE_API_URL` | Optional absolute API base for production builds (empty = same origin) |
| `VITE_DEFAULT_PROJECT` | Include this project in the sidebar when it has datasets but is missing from `GET /bigquery/v2/projects` |

## Development scripts

| Script | Description |
|--------|-------------|
| `npm run dev` | Vite dev server |
| `npm run build` | Typecheck + production bundle |
| `npm run preview` | Preview production build |
| `npm run test` | Vitest |
| `npm run test:e2e` | Playwright against Docker Compose stack |
| `npm run lint` | ESLint |

## CI

GitHub Actions builds and pushes the **nginx + static UI** Docker image (see `.github/workflows/docker-build.yml`) and runs **Playwright E2E tests** (see `.github/workflows/e2e.yml`).

## License

See the repository license file.
