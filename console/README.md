# BigQuery Emulator UI

React + TypeScript web UI for exploring BigQuery (emulator or real), modeled after a slim [Vite](https://vitejs.dev/) + [TanStack Query](https://tanstack.com/query) setup similar to internal Prospero web apps.

There is **no Go code in this repository**. The UI talks to the **BigQuery REST API** (`/bigquery/v2/*`) served by [`bigquery-emulator`](https://github.com/vantaboard/bigquery-emulator) on the default HTTP port **9050**. When the emulator runs with `--enable-sql-tools-api`, the query editor (M4) also uses the **SQL Tools API** (`/api/emulator/sql/*`) for format, parse, and completion.

## Features

- Browse projects, datasets, and tables (multi-project when the emulator supports discovery)
- View table metadata and schema
- Run SQL, format with BigQuery dialect, view tabular and JSON results
- Shareable URLs (query encoded in the query string)

See [docs/api-contract.md](./docs/api-contract.md) and [docs/rollout-checklist.md](./docs/rollout-checklist.md).

## Prerequisites

- **Node.js 22+** and npm
- **[direnv](https://direnv.net/)** (recommended) — loads [`.envrc`](.envrc) dev defaults when you enter the repo
- A running **bigquery-emulator** (or real BigQuery with credentials) exposing **`/bigquery/v2`** on the HTTP port you configure

## Run the emulator

Released image (compose / CI default):

```bash
docker run --rm -p 9050:9050 ghcr.io/vantaboard/bigquery-emulator:v0.5.0
```

For active emulator development, use a **local build** from a sibling checkout instead of waiting for GHCR releases — see [Develop against a local emulator build](#develop-against-a-local-emulator-build) below.

## Run the UI (development)

```bash
cd bigquery-emulator-ui
direnv allow          # once per checkout — loads .envrc
npm install
npm run dev
```

[Vite](https://vitejs.dev/) serves the app (default **5173**) and proxies **`/bigquery`** and **`/api/emulator`** to **`VITE_PROXY_TARGET`** (default `http://127.0.0.1:9050`, set in [`.envrc`](.envrc)).

## Develop against a local emulator build

Use this when you are changing [`bigquery-emulator`](https://github.com/vantaboard/bigquery-emulator) and want the UI on the same commit without waiting for a GitHub Actions release (which can take over an hour).

**Layout:** check out `bigquery-emulator` as a sibling of this repo (default path `../bigquery-emulator`).

### One-time setup (in `../bigquery-emulator`)

```bash
cd ../bigquery-emulator
mise install
task googlesql:fetch-prebuilt \
  URL=https://github.com/vantaboard/bigquery-emulator/releases/download/googlesql-prebuilt/v0.1.3+gs-36dd14aa0657/googlesql-prebuilt-linux-amd64-clang18-36dd14aa0657-v0.1.3.tar.gz \
  SHA256=be3b298245baef90aa8d0fd061be0f2d35cd0247ff3150e92b50962af09b44b4
task emulator:build-all
```

(URL/SHA256 match upstream `.github/workflows/release.yml`; update when that pin changes.)

### Daily loop (in `bigquery-emulator-ui`)

```bash
direnv allow            # loads VITE_PROXY_TARGET, VITE_DEFAULT_PROJECT, EMULATOR_ROOT
npm install
task dev:all            # local emulator + Vite on 5173
```

Or run the halves separately:

```bash
task emulator:run       # terminal 1 — seed from data/data.yaml, SQL Tools enabled
task dev                # terminal 2
```

**Verify:** `curl -fsS http://127.0.0.1:9050/healthz` and open `http://localhost:5173` — sidebar should show **`local-project`** / **`test-dataset`**.

### Rebuild after emulator changes

| Changed | Command | Then |
|---------|---------|------|
| Go gateway only | `task -d ../bigquery-emulator emulator:build` | restart `task emulator:run` or `task dev:all` |
| C++ engine | `task -d ../bigquery-emulator emulator:build-engine:bazel` | restart |
| Both / unsure | `task emulator:build` | restart |

**Task reference:** `task emulator:check` (binaries present), `task emulator:build`, `task emulator:run`, `task dev`, `task dev:all`. Override paths with `EMULATOR_ROOT`, seed with `SEED_FILE`, port with `EMULATOR_HTTP_PORT`.

**Alternative (slower, nginx parity):** build a local Docker image from the emulator checkout (`docker compose up --build` in `bigquery-emulator`) instead of pulling `ghcr.io/vantaboard/bigquery-emulator:v0.5.0`.

## Production build

```bash
npm run build
npm run preview
```

For a static build that talks to the API on another origin, set **`VITE_API_URL`** at build time (e.g. `https://api.example.com`).

## Docker

`docker compose up` runs [`ghcr.io/vantaboard/bigquery-emulator:v0.5.0`](https://github.com/vantaboard/bigquery-emulator) as **`bigquery`** and the static UI (**`bq-ui`**) on **8080**. Nginx proxies **`/bigquery`** and **`/api/emulator`** to **`http://bigquery:9050`**. Sample data is loaded from **`data/data.yaml`**. The emulator service passes **`--sql-tools-api-allow-remote`**; the release image also enables SQL Tools via its entrypoint shim.

## E2E testing

End-to-end tests use the same emulator image with a smaller fixture in **`e2e/fixtures/seed.yaml`** via **`docker-compose.e2e.yaml`**:

```bash
npm install
npm run test:e2e          # starts stack, runs Playwright, tears down
npm run test:e2e:ui       # Playwright UI mode (stack must be up)
npm run e2e:up            # start stack only (UI on 8080)
npm run e2e:down          # stop stack and remove volumes
```

The E2E stack is defined in **`docker-compose.e2e.yaml`**. Seed data lives in **`e2e/fixtures/seed.yaml`**. CI runs the same suite via **`.github/workflows/e2e.yml`** (pinned **`v0.5.0`** image).

**Local emulator (pre-release):** run Playwright against a native build instead of the pinned image:

```bash
task e2e:local          # starts local emulator + Vite, runs Playwright, tears down
# or: npm run test:e2e:local
```

CI is unchanged — it still uses the released GHCR image for reproducibility.

## Environment

Dev defaults live in **[`.envrc`](.envrc)** (loaded by direnv). Override locally in **`.envrc.local`** (gitignored) or export before `direnv reload`.

| Variable | Purpose |
|----------|---------|
| `VITE_PROXY_TARGET` | Dev: Vite proxy target for `/bigquery` and `/api/emulator` (default `http://127.0.0.1:9050`) |
| `VITE_API_URL` | Optional absolute API base for production builds (empty = same origin) |
| `VITE_DEFAULT_PROJECT` | Include this project in the sidebar when it has datasets but is missing from `GET /bigquery/v2/projects` (default `local-project`) |
| `VITE_SQL_TOOLS_TOKEN` | Optional: `X-BigQuery-Emulator-SqlTools-Token` for remote SQL Tools access when the emulator requires a token |
| `EMULATOR_ROOT` | Taskfile: path to local `bigquery-emulator` checkout (default `../bigquery-emulator`) |

## Development scripts

| Script | Description |
|--------|-------------|
| `npm run dev` | Vite dev server |
| `npm run dev:local-emulator` | Local emulator + Vite (`task dev:all`) |
| `npm run build` | Typecheck + production bundle |
| `npm run preview` | Preview production build |
| `npm run test` | Vitest |
| `npm run test:e2e` | Playwright against Docker Compose stack (pinned release image) |
| `npm run test:e2e:local` | Playwright against native emulator + Vite |
| `npm run lint` | ESLint |

| Task | Description |
|------|-------------|
| `task dev:all` | Local emulator (background) + Vite |
| `task emulator:run` | Local emulator only (foreground) |
| `task emulator:build` | Build sibling `bigquery-emulator` checkout |
| `task e2e:local` | E2E against native emulator |

## CI

GitHub Actions builds and pushes the **nginx + static UI** Docker image (see `.github/workflows/docker-build.yml`) and runs **Playwright E2E tests** (see `.github/workflows/e2e.yml`).

## License

See the repository license file.
